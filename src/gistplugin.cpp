#include "gistplugin.h"
#include "gistpluginconstants.h"
#include "gistmanager.h"
#include "settings.h"
#include "optionspage.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/idocument.h>
#include <coreplugin/messagemanager.h>
#include <texteditor/texteditor.h>
#include <texteditor/textdocument.h>

#include <QtPlugin>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QMenu>

using namespace Core;
using namespace TextEditor;

namespace GitHubGist {
namespace Internal {

GistPlugin::GistPlugin() :
    m_settings(new Settings),
    m_gistManager(0),
    m_optionsPage(0)
{
    // Create your members
}

GistPlugin::~GistPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
}

bool GistPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    m_settings->load(ICore::settings());
    m_gistManager = new GistManager(m_settings, this);

    connect(m_gistManager, &GistManager::apiError, this, &GistPlugin::showMessage);
    connect(m_gistManager, &GistManager::gistPosted, this, &GistPlugin::gistCreated);

    createMenu();
    createOptionsPage();

    return true;
}

void GistPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag GistPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    return SynchronousShutdown;
}

static inline void textFromCurrentEditor(QString *text, QString *fileName = 0)
{
    IEditor *editor = EditorManager::currentEditor();
    if (!editor)
        return;

    const IDocument *document = editor->document();

    if (fileName) {
        *fileName = editor->document()->filePath().fileName();
    }

    QString data;
    if (const BaseTextEditor *textEditor = qobject_cast<const BaseTextEditor *>(editor))
        data = textEditor->selectedText();
    if (data.isEmpty()) {
        if (auto textDocument = qobject_cast<const TextDocument *>(document)) {
            data = textDocument->plainText();
        } else {
            const QVariant textV = document->property("plainText"); // Diff Editor.
            if (textV.type() == QVariant::String)
                data = textV.toString();
        }
    }
    if (!data.isEmpty()) {
        *text = data;
    }
}

static inline void fixSpecialCharacters(QString &data)
{
    QChar *uc = data.data();
    QChar *e = uc + data.size();

    for (; uc != e; ++uc) {
        switch (uc->unicode()) {
        case 0xfdd0: // QTextBeginningOfFrame
        case 0xfdd1: // QTextEndOfFrame
        case QChar::ParagraphSeparator:
        case QChar::LineSeparator:
            *uc = QLatin1Char('\n');
            break;
        case QChar::Nbsp:
            *uc = QLatin1Char(' ');
            break;
        default:
            break;
        }
    }
}

void GistPlugin::createGist()
{
    QString text;
    QString fileName;

    textFromCurrentEditor(&text, &fileName);
    fixSpecialCharacters(text);

    if (text.isEmpty() || fileName.isEmpty()) {
        showMessage(tr("Gist not created. None of the selected text."));
        return;
    }

    bool publicFlag = sender()->property("public").toBool();
    m_gistManager->postGist(text, fileName, fileName, publicFlag);
}

void GistPlugin::createMenu()
{
    QAction *createPublicAction = new QAction(QIcon(QLatin1String(":/images/gist.png")),
                                              tr("Create gist"), this);
    createPublicAction->setProperty("public", true);
    Command *createPublicGistCmd = ActionManager::registerAction(createPublicAction,
                                                                 Constants::CREATE_PUBLIC_ACTION_ID,
                                                                 Context(Core::Constants::C_EDIT_MODE));
    createPublicGistCmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+G")));
    connect(createPublicAction, &QAction::triggered, this, &GistPlugin::createGist);

    QAction *createSecretAction = new QAction(QIcon(QLatin1String(":/images/gist-secret.png")),
                                              tr("Create secret gist"), this);
    createSecretAction->setProperty("public", false);
    Command *createSecretGistCmd = ActionManager::registerAction(createSecretAction,
                                                                 Constants::CREATE_SECRET_ACTION_ID,
                                                                 Context(Core::Constants::C_EDIT_MODE));
    connect(createSecretAction, &QAction::triggered, this, &GistPlugin::createGist);

    ActionContainer *gistsMenu = ActionManager::createMenu(Constants::GIST_TOOLS_MENU_ID);
    gistsMenu->menu()->setTitle(tr("GitHub Gist"));
    gistsMenu->addAction(createPublicGistCmd);
    gistsMenu->addAction(createSecretGistCmd);
    ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(gistsMenu);
}

void GistPlugin::createOptionsPage()
{
    m_optionsPage = new OptionsPage(m_settings, this);
    addAutoReleasedObject(m_optionsPage);
}

void GistPlugin::showMessage(const QString &message)
{
    MessageManager::write(message);
}

void GistPlugin::gistCreated(const QString &name, const QString &url)
{
    if (m_settings->autoCopyLink) {
        QApplication::clipboard()->setText(url);
    }
    showMessage(tr("Gist \"%1\" posted: ").arg(name) + url);
}

} // namespace Internal
} // namespace GitHubGist
