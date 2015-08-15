/***************************************************************************
 *   Copyright (C) 2015 by Darcy Shen <sadhen@zoho.com>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "marknote.h"

#include <KLocale>
#include <KConfig>
#include <KXMLGUIFactory>
#include <KActionCollection>
#include <KStandardAction>
#include <KRecentFilesAction>
#include <KFileDialog>
#include <KUrl>
#include <KWebView>
#include <KMenuBar>
#include <KShortcut>

#include <QKeySequence>

MarkNote::MarkNote(QWidget* parent)
    : KXmlGuiWindow(parent)
    , isPreview(false)
    , m_recentFiles(0)
{
    KAction* previewAction = actionCollection()->addAction("file_preview", this, SLOT(togglePreview()));
    previewAction->setIcon(KIcon("document-preview"));
    previewAction->setText(i18n("Preview"));
    previewAction->setCheckable(true);
    previewAction->setShortcut(QKeySequence("F8"));

    m_view = new MainView(parent, previewAction);
    m_note = m_view->note;
    setupAction();
    setupUI();
    setupConnect();
}

void MarkNote::setupAction()
{
    KStandardAction::openNew(this, SLOT(newNote()), actionCollection());
    KStandardAction::close(this, SLOT(close()), actionCollection());

    KAction* oneColAction = actionCollection()->addAction("win_onecol", m_view, SLOT(oneColView()));
    KAction* twoColAction = actionCollection()->addAction("win_twocol", m_view, SLOT(twoColView()));
    KAction* threeColAction = actionCollection()->addAction("win_threecol", m_view, SLOT(threeColView()));
    m_recentFiles = KStandardAction::openRecent(m_note, SLOT(openUrl(KUrl)), this);
    actionCollection()->addAction(m_recentFiles->objectName(), m_recentFiles);
    m_recentFiles->setWhatsThis(i18n("This lists files which you have opened recently, and allows you to easily open them again."));

    oneColAction->setText(i18n("One Column"));
    twoColAction->setText(i18n("Two Column"));
    threeColAction->setText(i18n("Three Column"));

    oneColAction->setIcon(KIcon("view-split-top-bottom"));
    twoColAction->setIcon(KIcon("view-split-left-right"));
    threeColAction->setIcon(KIcon("view-file-columns"));

    KAction* terminalAction = actionCollection()->addAction("toggle_terminal", m_view, SLOT(toggleTerminal()));
    KShortcut terminalShortcut = terminalAction->shortcut();
    terminalShortcut.setPrimary(QKeySequence("F4"));
    terminalAction->setShortcut(terminalShortcut);
}

void MarkNote::setupUI()
{
    setCentralWidget(m_view);
    setupGUI(QSize(500,600), Default, "marknote.rc");
    guiFactory()->addClient(m_view->getEditor());
    setStandardToolBarMenuEnabled(true);
    setAutoSaveSettings();
    readConfig();
}

void MarkNote::setupConnect()
{
    connect(m_note, SIGNAL(modifiedChanged(KTextEditor::Document*)),
            this, SLOT(updateCaption()));
    connect(m_note, SIGNAL(documentUrlChanged(KTextEditor::Document*)),
            this, SLOT(slotDocumentUrlChanged()));
    connect(m_note, SIGNAL(textChanged(KTextEditor::Document *)),
            this, SLOT(updateCaptionModified()));
}

void MarkNote::newNote()
{
    m_view->newNote();
}

void MarkNote::updateCaptionModified()
{
    setCaption(m_note->url().fileName() + " [modified]- MarkNote");
}

void MarkNote::updateCaption()
{
    setCaption(m_note->url().fileName() + " - MarkNote");
}

void MarkNote::slotDocumentUrlChanged()
{
    setCaption(m_note->url().fileName() + " - MarkNote");
    if (!m_note->url().isEmpty())
        m_recentFiles->addUrl(m_note->url());
}

void MarkNote::togglePreview()
{
    if (isPreview)
        isPreview = m_view->unpreview();
    else
        isPreview = m_view->preview();
    actionCollection()->action("file_preview")->setChecked(isPreview);
}

//common config
void MarkNote::readConfig(KSharedConfigPtr config)
{
    KConfigGroup cfg(config, "General Options");

    m_recentFiles->loadEntries(config->group("Recent Files"));
}

void MarkNote::writeConfig(KSharedConfigPtr config)
{
    KConfigGroup generalOptions(config, "General Options");

    m_recentFiles->saveEntries(KConfigGroup(config, "Recent Files"));

    config->sync();
}

//config file
void MarkNote::readConfig()
{
    readConfig(KSharedConfig::openConfig());
}

void MarkNote::writeConfig()
{
    writeConfig(KSharedConfig::openConfig());
}

void MarkNote::readProperties(const KConfigGroup &config)
{
    readConfig();
    Q_UNUSED(config);
}

void MarkNote::saveProperties(KConfigGroup &cg)
{
    writeConfig();
    Q_UNUSED(cg);
}


MarkNote::~MarkNote()
{
    writeConfig();
}

#include "marknote.moc"