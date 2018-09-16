/*
 ===============================================================================
 Name        : DuplicatesTreeView.hpp
 Author      : NeatDecisions
 Version     :
 Copyright   : Copyright © 2018 Neat Decisions. All rights reserved.
 Description : Detwinner
 ===============================================================================
 */

#ifndef UI_DUPLICATESTREEVIEW_HPP_
#define UI_DUPLICATESTREEVIEW_HPP_

#include <gtkmm.h>

#include <gtkmm/treestore.h>
#include <logic/CommonDataTypes.hpp>
#include <callbacks/IDeferredAction.hpp>
#include <callbacks/IDuplicateReceiver.hpp>
#include <tools/AbstractFileDeleter.hpp>


namespace detwinner {
namespace ui {


class DuplicatesTreeView : public Gtk::TreeView
{
public:
	enum class Mode_t
	{
		Normal,
		Images
	};

	struct DuplicateStats_t
	{
		unsigned int groupCount = 0;
		unsigned int fileCount = 0;
		unsigned long long totalSize = 0;
		unsigned long long wastedSize = 0;
	};

	DuplicatesTreeView();
	virtual ~DuplicatesTreeView() noexcept override;

	void setMode(Mode_t mode);
	bool empty() const;
	void clear();
	bool atLeastOneTopLevelItemChecked() const;
	DuplicateStats_t calculateStats() const;

	// sorting methods
	void sortByTotalSize(bool asc);
	void sortBySingleFileSize(bool asc);
	void sortByNumberOfFiles(bool asc);

	// smart selection methods
	callbacks::IDeferredAction::Ptr_t smartSelect_KeepFirstInGroup();
	callbacks::IDeferredAction::Ptr_t smartSelect_KeepLastInGroup();

	callbacks::IDeferredAction::Ptr_t smartSelect_KeepEarliestModified();
	callbacks::IDeferredAction::Ptr_t smartSelect_KeepLatestModified();

	callbacks::IDeferredAction::Ptr_t smartSelect_KeepShortestName();
	callbacks::IDeferredAction::Ptr_t smartSelect_KeepLongestName();

	callbacks::IDeferredAction::Ptr_t smartSelect_KeepShortestPath();
	callbacks::IDeferredAction::Ptr_t smartSelect_KeepLongestPath();

	callbacks::IDeferredAction::Ptr_t smartSelect_KeepLowestResolution();
	callbacks::IDeferredAction::Ptr_t smartSelect_KeepHighestResolution();

	// bulk selection methods
	callbacks::IDeferredAction::Ptr_t selectAll();
	callbacks::IDeferredAction::Ptr_t clearSelection();
	callbacks::IDeferredAction::Ptr_t invertSelection();

	// deletion methods
	callbacks::IDeferredAction::Ptr_t deletePermanently();
	callbacks::IDeferredAction::Ptr_t deleteToTrash();
	callbacks::IDeferredAction::Ptr_t deleteToBackupFolder(const std::string & folder, Gtk::Window * dialogParent);

	callbacks::IDuplicateReceiver::Ptr_t createPopulationDelegate();

	// row selection signal
	using signal_duplicate_selected = sigc::signal<void, Glib::ustring, Glib::ustring>;
	signal_duplicate_selected on_duplicate_selected();

	using signal_stats_changed = sigc::signal<void>;
	signal_stats_changed on_stats_changed();

protected:
	// row selection handler
	virtual void on_cursor_changed() override;
	virtual void on_row_activated(const Gtk::TreeModel::Path & path, Gtk::TreeViewColumn * treeColumn) override;
	virtual void on_size_allocate(Gtk::Allocation & s) override;
	virtual void on_hide() override;
	virtual bool on_button_press_event(GdkEventButton * button_event) override;

private:
	enum class CheckState_t
	{
		Checked,
		Unchecked,
		Mixed
	};

	struct FileModelColumn : public Gtk::TreeModel::ColumnRecord
	{
		Gtk::TreeModelColumn<bool> locked;
		Gtk::TreeModelColumn<CheckState_t> checkState;
		Gtk::TreeModelColumn<Glib::ustring> filePath;
		Gtk::TreeModelColumn<unsigned long long> fileSize;
		Gtk::TreeModelColumn<Glib::DateTime> updateTime;
		Gtk::TreeModelColumn<unsigned int> width;
		Gtk::TreeModelColumn<unsigned int> height;
		FileModelColumn() { add(checkState); add(locked); add(filePath); add(fileSize); add(updateTime); add(width); add(height);}
	};

	// checkbox handler
	virtual void on_cell_toggled(const Glib::ustring & path);

	// check getter/setter
	CheckState_t getCheck(const Gtk::TreeIter & iter) const;
	bool setCheck(const Gtk::TreeIter & iter, CheckState_t checkState, bool noUp, bool noDown);

	// cell rendering handles
	void on_render_filename(Gtk::CellRenderer * cellRenderer, const Gtk::TreeModel::iterator & iter);
	void on_render_path(Gtk::CellRenderer * cellRenderer, const Gtk::TreeModel::iterator & iter);
	void on_render_lock(Gtk::CellRenderer * cellRenderer, const Gtk::TreeModel::iterator & iter);
	void on_render_size(Gtk::CellRenderer * cellRenderer, const Gtk::TreeModel::iterator & iter);
	void on_render_resolution(Gtk::CellRenderer * cellRenderer, const Gtk::TreeModel::iterator & iter);
	void on_render_update_time(Gtk::CellRenderer * cellRenderer, const Gtk::TreeModel::iterator & iter);
	void on_render_toggle(Gtk::CellRenderer * cellRenderer, const Gtk::TreeModel::iterator & iter);
	void on_render_icon(Gtk::CellRenderer * cellRenderer, const Gtk::TreeModel::iterator & iter);

	// tree sorting functions
	int sort_func_total_size(const Gtk::TreeModel::iterator &, const Gtk::TreeModel::iterator &) const;
	int sort_func_single_file_size(const Gtk::TreeModel::iterator &, const Gtk::TreeModel::iterator &) const;
	int sort_func_number_of_files(const Gtk::TreeModel::iterator &, const Gtk::TreeModel::iterator &) const;

	// context menu handling functions
	void on_duplicate_file_open() const;
	void on_duplicate_file_open_folder() const;
	void on_group_smart_select_keep_first();
	void on_group_smart_select_keep_last();
	void on_group_smart_select_keep_earliest();
	void on_group_smart_select_keep_latest();
	void on_group_smart_select_keep_shortestname();
	void on_group_smart_select_keep_longestname();
	void on_group_smart_select_keep_shortestpath();
	void on_group_smart_select_keep_longestpath();
	void on_group_smart_select_keep_lowestresolution();
	void on_group_smart_select_keep_highestresolution();
	void on_group_select_all();
	void on_group_clear_selection();
	void on_group_invert_selection();
	void on_group_exclude();

	Glib::ustring getPreviewPath(const Gtk::TreeIter & iter) const;
	Glib::ustring extractFullPath(const Gtk::TreeIter & iter) const;
	bool isDuplicateFileIter(const Gtk::TreeIter & iter) const;
	Gtk::TreeIter getSelectedIter() const;

	class PopulationDelegate : public callbacks::IDuplicateReceiver
	{
	public:
		explicit PopulationDelegate(DuplicatesTreeView & tree);
		virtual callbacks::IDeferredAction::Ptr_t populate(logic::DuplicatesList_t && container) override;
	private:
		DuplicatesTreeView & m_tree;
	};

	class TreeAction : public callbacks::IDeferredAction
	{
	public:
		explicit TreeAction(DuplicatesTreeView & tree);
		virtual double getProgress() const override;
	protected:
		DuplicatesTreeView & m_tree;
		std::size_t m_totalItems;
		std::size_t m_currentItem;
		Gtk::TreeModel::iterator m_iter;
	};

	struct ISmartSelector
	{
		using Ptr_t = std::shared_ptr<ISmartSelector>;
		virtual ~ISmartSelector() = default;
		virtual void select(DuplicatesTreeView & tree, Gtk::TreeIter iter) = 0;
	};

	class SmartSelector_t : public ISmartSelector
	{
	public:
		using SelectFunc_t = std::function<bool(const Gtk::TreeModel::iterator&, const Gtk::TreeModel::iterator&)>;
		using IgnoreFunc_t = std::function<bool(const Gtk::TreeModel::iterator&)>;
		explicit SmartSelector_t(const SelectFunc_t & selectFunc, const IgnoreFunc_t & ignoreFunc = [](const Gtk::TreeModel::iterator&){ return false; });
		virtual void select(DuplicatesTreeView & tree, Gtk::TreeIter iter) override;
	private:
		SelectFunc_t m_selectFunc;
		IgnoreFunc_t m_ignoreFunc;
	};

	class TreeSelect_Smart : public TreeAction
	{
	public:
		TreeSelect_Smart(const ISmartSelector::Ptr_t & smartSelector, DuplicatesTreeView & tree);
		virtual bool processNext() override;
	private:
		ISmartSelector::Ptr_t m_smartSelector;
	};

	class TreeSelect_Bulk_t : public TreeAction
	{
	public:
		using SelectFunc_t = std::function<CheckState_t(const Gtk::TreeModel::iterator&)>;
		TreeSelect_Bulk_t(const SelectFunc_t & selectFunc, DuplicatesTreeView & tree);
		virtual bool processNext() override;
	private:
		SelectFunc_t m_selectFunc;
	};

	class TreePopulateAction : public callbacks::IDeferredAction
	{
	public:
		explicit TreePopulateAction(DuplicatesTreeView & tree, logic::DuplicatesList_t && values);
		virtual ~TreePopulateAction() noexcept override;
		virtual double getProgress() const override;
		virtual bool processNext() override;
	private:
		void beginBatch();
		void endBatch();
		DuplicatesTreeView & m_tree;
		logic::DuplicatesList_t m_values;
		std::size_t m_currentItem;
		bool m_batchStarted;
	};

	class TreeDeleteAction : public TreeAction
	{
	public:
		explicit TreeDeleteAction(DuplicatesTreeView & tree, tools::AbstractFileDeleter::Ptr_t fileDeleter);
		virtual bool processNext() override;
	private:
		tools::AbstractFileDeleter::Ptr_t m_fileDeleter;
	};

	ISmartSelector::Ptr_t createSmartSelector_KeepFirstInGroup();
	ISmartSelector::Ptr_t createSmartSelector_KeepLastInGroup();
	ISmartSelector::Ptr_t createSmartSelector_KeepEarliestModified();
	ISmartSelector::Ptr_t createSmartSelector_KeepLatestModified();
	ISmartSelector::Ptr_t createSmartSelector_KeepShortestName();
	ISmartSelector::Ptr_t createSmartSelector_KeepLongestName();
	ISmartSelector::Ptr_t createSmartSelector_KeepShortestPath();
	ISmartSelector::Ptr_t createSmartSelector_KeepLongestPath();
	ISmartSelector::Ptr_t createSmartSelector_KeepLowestResolution();
	ISmartSelector::Ptr_t createSmartSelector_KeepHighestResolution();

	std::unique_ptr<Gtk::Menu> loadMenu(const Glib::ustring & name);

	Gtk::CellRendererToggle m_toggleRenderer;
	Gtk::CellRendererPixbuf m_lockRenderer;
	Gtk::CellRendererText m_sizeRenderer;
	Gtk::CellRendererText m_resolutionRenderer;
	Gtk::CellRendererText m_fileNameRenderer;
	Gtk::CellRendererText m_pathRenderer;
	Gtk::CellRendererText m_updateTimeRenderer;
	Gtk::CellRendererPixbuf m_iconRenderer;

	FileModelColumn m_columns;
	Glib::RefPtr<Gtk::TreeStore> m_store;
	Glib::RefPtr<Gtk::Builder> m_refBuilder;
	std::unique_ptr<Gtk::Menu> m_refDuplicateFileMenu;
	std::unique_ptr<Gtk::Menu> m_refDuplicateGroupMenu;
	Glib::RefPtr<Gio::SimpleActionGroup> m_refActionGroup;

	signal_duplicate_selected m_signalDuplicateSelected;
	signal_stats_changed m_signalStatsChanged;

	Mode_t m_mode;
	bool m_adapted;

	Gtk::TreeViewColumn * m_columnResolution;
};

}}

#endif /* UI_DUPLICATESTREEVIEW_HPP_ */
