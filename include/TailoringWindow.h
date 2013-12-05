/*
 * Copyright 2013 Red Hat Inc., Durham, North Carolina.
 * All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *      Martin Preisler <mpreisle@redhat.com>
 */

#ifndef SCAP_WORKBENCH_TAILORING_WINDOW_H_
#define SCAP_WORKBENCH_TAILORING_WINDOW_H_

#include "ForwardDecls.h"

#include <QDialog>
#include <QUndoCommand>
#include <QUndoStack>

extern "C"
{
#include <xccdf_benchmark.h>
#include <xccdf_policy.h>
}

#include "ui_TailoringWindow.h"
#include "ui_ProfilePropertiesDockWidget.h"
#include "ui_XCCDFItemPropertiesDockWidget.h"

class TailoringWindow;

/**
 * @brief
 */
class ProfilePropertiesDockWidget : public QDockWidget
{
    Q_OBJECT

    public:
        ProfilePropertiesDockWidget(TailoringWindow* window, QWidget* parent = 0);
        virtual ~ProfilePropertiesDockWidget();

        void refresh();

    protected slots:
        void profileTitleChanged(const QString& currentTitle);

    protected:
        bool mUndoRedoInProgress;

        /// UI designed in Qt Designer
        Ui_ProfilePropertiesDockWidget mUI;

        TailoringWindow* mWindow;
};

/**
 * @brief Provides reference about currently selected XCCDF item
 */
class XCCDFItemPropertiesDockWidget : public QDockWidget
{
    Q_OBJECT

    public:
        XCCDFItemPropertiesDockWidget(QWidget* parent = 0);
        virtual ~XCCDFItemPropertiesDockWidget();

        void setXccdfItem(struct xccdf_item* item);
        void refresh();

    protected:
        /// UI designed in Qt Designer
        Ui_XCCDFItemPropertiesDockWidget mUI;

        struct xccdf_item* mXccdfItem;
};

/**
 * @brief Stores info about one selection or deselection of an XCCDF item
 */
class XCCDFItemSelectUndoCommand : public QUndoCommand
{
    public:
        XCCDFItemSelectUndoCommand(TailoringWindow* window, QTreeWidgetItem* item, bool newSelect);
        virtual ~XCCDFItemSelectUndoCommand();

        virtual int id() const;

        virtual void redo();
        virtual void undo();

    private:
        TailoringWindow* mWindow;

        QTreeWidgetItem* mTreeItem;
        /// selection state after this undo command is "redone" (applied)
        bool mNewSelect;
};

/**
 * @brief Stores info title change in XCCDF profile
 */
class ProfileTitleChangeUndoCommand : public QUndoCommand
{
    public:
        ProfileTitleChangeUndoCommand(TailoringWindow* window, const QString& oldTitle, const QString& newTitle);
        virtual ~ProfileTitleChangeUndoCommand();

        virtual int id() const;

        virtual void redo();
        virtual void undo();

        virtual bool mergeWith(const QUndoCommand *other);

    private:
        TailoringWindow* mWindow;

        QString mOldTitle;
        QString mNewTitle;
};

/**
 * @brief Tailors given profile by editing it directly
 *
 * If you want to inherit a profile and tailor that, create a new profile,
 * set up the inheritance and then pass the new profile to this class.
 */
class TailoringWindow : public QMainWindow
{
    Q_OBJECT

    public:
        TailoringWindow(struct xccdf_policy* policy, struct xccdf_benchmark* benchmark, MainWindow* parent = 0);
        virtual ~TailoringWindow();

        void setItemSelected(struct xccdf_item* xccdfItem, bool selected);
        void synchronizeTreeItem(QTreeWidgetItem* treeItem, struct xccdf_item* xccdfItem, bool recursive);

        QString getProfileID() const;

        void setProfileTitle(const QString& title);
        QString getProfileTitle() const;

        void setProfileTitleWithUndoCommand(const QString& newTitle);

        void refreshProfileDockWidget();

    protected:
        /// Reimplemented to refresh selected rules in the parent main window
        virtual void closeEvent(QCloseEvent * event);

        MainWindow* mParentMainWindow;

        /// if > 0, ignore itemChanged signals, these would just excessively add selects and bloat memory
        unsigned int mSynchronizeItemLock;

        /// UI designed in Qt Designer
        Ui_TailoringWindow mUI;

        ProfilePropertiesDockWidget* mProfilePropertiesDockWidget;
        XCCDFItemPropertiesDockWidget* mItemPropertiesDockWidget;

        struct xccdf_policy* mPolicy;
        struct xccdf_profile* mProfile;
        struct xccdf_benchmark* mBenchmark;

        QUndoStack mUndoStack;

    protected slots:
        void itemSelectionChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
        void itemChanged(QTreeWidgetItem* item, int column);
};

#endif
