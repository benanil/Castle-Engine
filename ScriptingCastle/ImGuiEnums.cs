
// Flags for ImGui::Begin()
public enum ImGuiWindowFlags
{
    None = 0,
    NoTitleBar = 1 << 0,   // Disable title-bar
    NoResize = 1 << 1,   // Disable user resizing with the lower-right grip
    NoMove = 1 << 2,   // Disable user moving the window
    NoScrollbar = 1 << 3,   // Disable scrollbars (window can still scroll with mouse or programmatically)
    NoScrollWithMouse = 1 << 4,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
    NoCollapse = 1 << 5,   // Disable user collapsing window by double-clicking on it. Also referred to as "window menu button" within a docking node.
    AlwaysAutoResize = 1 << 6,   // Resize every window to its content every frame
    NoBackground = 1 << 7,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
    NoSavedSettings = 1 << 8,   // Never load/save settings in .ini file
    NoMouseInputs = 1 << 9,   // Disable catching mouse, hovering test with pass through.
    MenuBar = 1 << 10,  // Has a menu-bar
    HorizontalScrollbar = 1 << 11,  // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
    NoFocusOnAppearing = 1 << 12,  // Disable taking focus when transitioning from hidden to visible state
    NoBringToFrontOnFocus = 1 << 13,  // Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)
    AlwaysVerticalScrollbar = 1 << 14,  // Always show vertical scrollbar (even if ContentSize.y < Size.y)
    AlwaysHorizontalScrollbar = 1 << 15,  // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
    AlwaysUseWindowPadding = 1 << 16,  // Ensure child windows without border uses style.WindowPadding (ignored by default for non-bordered child windows, because more convenient)
    NoNavInputs = 1 << 18,  // No gamepad/keyboard navigation within the window
    NoNavFocus = 1 << 19,  // No focusing toward this window with gamepad/keyboard navigation (e.g. skipped by CTRL+TAB)
    UnsavedDocument = 1 << 20,  // Display a dot next to the title. When used in a tab/docking context, tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
    NoDocking = 1 << 21,  // Disable docking of this window

    NoNav = NoNavInputs | NoNavFocus,
    NoDecoration = NoTitleBar | NoResize | NoScrollbar | NoCollapse,
    NoInputs = NoMouseInputs | NoNavInputs | NoNavFocus,

    // [Internal]
    NavFlattened = 1 << 23,  // [BETA] Allow gamepad/keyboard navigation to cross over parent border to this child (only use on child that have no scrolling!)
    ChildWindow = 1 << 24,  // Don't use! For internal use by BeginChild()
    Tooltip = 1 << 25,  // Don't use! For internal use by BeginTooltip()
    Popup = 1 << 26,  // Don't use! For internal use by BeginPopup()
    Modal = 1 << 27,  // Don't use! For internal use by BeginPopupModal()
    ChildMenu = 1 << 28,  // Don't use! For internal use by BeginMenu()
    DockNodeHost = 1 << 29   // Don't use! For internal use by Begin()/NewFrame()

    // [Obsolete]
    //ImGuiWindowFlags_ResizeFromAnySide    = 1 << 17,  // --> Set io.ConfigWindowsResizeFromEdges=true and make sure mouse cursors are supported by backend (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors)
};

// Flags for ImGui::InputText()
public enum ImGuiInputTextFlags
{
    None = 0,
    CharsDecimal = 1 << 0,   // Allow 0123456789.+-*/
    CharsHexadecimal = 1 << 1,   // Allow 0123456789ABCDEFabcdef
    CharsUppercase = 1 << 2,   // Turn a..z into A..Z
    CharsNoBlank = 1 << 3,   // Filter out spaces, tabs
    AutoSelectAll = 1 << 4,   // Select entire text when first taking mouse focus
    EnterReturnsTrue = 1 << 5,   // Return 'true' when Enter is pressed (as opposed to every time the value was modified). Consider looking at the IsItemDeactivatedAfterEdit() function.
    CallbackCompletion = 1 << 6,   // Callback on pressing TAB (for completion handling)
    CallbackHistory = 1 << 7,   // Callback on pressing Up/Down arrows (for history handling)
    CallbackAlways = 1 << 8,   // Callback on each iteration. User code may query cursor position, modify text buffer.
    CallbackCharFilter = 1 << 9,   // Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
    AllowTabInput = 1 << 10,  // Pressing TAB input a '\t' character into the text field
    CtrlEnterForNewLine = 1 << 11,  // In multi-line mode, unfocus with Enter, add new line with Ctrl+Enter (default is opposite: unfocus with Ctrl+Enter, add line with Enter).
    NoHorizontalScroll = 1 << 12,  // Disable following the cursor horizontally
    AlwaysOverwrite = 1 << 13,  // Overwrite mode
    ReadOnly = 1 << 14,  // Read-only mode
    Password = 1 << 15,  // Password mode, display all characters as '*'
    NoUndoRedo = 1 << 16,  // Disable undo/redo. Note that input text owns the text data while active, if you want to provide your own undo/redo stack you need e.g. to call ClearActiveID().
    CharsScientific = 1 << 17,  // Allow 0123456789.+-*/eE (Scientific notation input)
    CallbackResize = 1 << 18,  // Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow. Notify when the string wants to be resized (for string types which hold a cache of their Size). You will be provided a new BufSize in the callback and NEED to honor it. (see misc/cpp/imgui_stdlib.h for an example of using this)
    CallbackEdit = 1 << 19   // Callback on any edit (note that InputText() already returns true on edit, the callback is useful mainly to manipulate the underlying buffer while focus is active)
};

// Flags for ImGui::TreeNodeEx(), ImGui::CollapsingHeader*()
public enum ImGuiTreeNodeFlags
{
    None = 0,
    Selected = 1 << 0,   // Draw as selected
    Framed = 1 << 1,   // Draw frame with background (e.g. for CollapsingHeader)
    AllowItemOverlap = 1 << 2,   // Hit testing to allow subsequent widgets to overlap this one
    NoTreePushOnOpen = 1 << 3,   // Don't do a TreePush() when open (e.g. for CollapsingHeader) = no extra indent nor pushing on ID stack
    NoAutoOpenOnLog = 1 << 4,   // Don't automatically and temporarily open node when Logging is active (by default logging will automatically open tree nodes)
    DefaultOpen = 1 << 5,   // Default node to be open
    OpenOnDoubleClick = 1 << 6,   // Need double-click to open node
    OpenOnArrow = 1 << 7,   // Only open when clicking on the arrow part. If ImGuiTreeNodeFlags_OpenOnDoubleClick is also set, single-click arrow or double-click all box to open.
    Leaf = 1 << 8,   // No collapsing, no arrow (use as a convenience for leaf nodes).
    Bullet = 1 << 9,   // Display a bullet instead of arrow
    FramePadding = 1 << 10,  // Use FramePadding (even for an unframed text node) to vertically align text baseline to regular widget height. Equivalent to calling AlignTextToFramePadding().
    SpanAvailWidth = 1 << 11,  // Extend hit box to the right-most edge, even if not framed. This is not the default in order to allow adding other items on the same line. In the future we may refactor the hit system to be front-to-back, allowing natural overlaps and then this can become the default.
    SpanFullWidth = 1 << 12,  // Extend hit box to the left-most and right-most edges (bypass the indented area).
    NavLeftJumpsBackHere = 1 << 13,  // (WIP) Nav: left direction may move to this TreeNode() from any of its child (items submitted between TreeNode and TreePop)
    CollapsingHeader = Framed | NoTreePushOnOpen | NoAutoOpenOnLog
};

// Flags for OpenPopup*(), BeginPopupContext*(), IsPopupOpen() functions.
// - To be backward compatible with older API which took an 'int mouse_button = 1' argument, we need to treat
//   small flags values as a mouse button index, so we encode the mouse button in the first few bits of the flags.
//   It is therefore guaranteed to be legal to pass a mouse button index in ImGuiPopupFlags.
// - For the same reason, we exceptionally default the ImGuiPopupFlags argument of BeginPopupContextXXX functions to 1 instead of 0.
//   IMPORTANT: because the default parameter is 1 (==ImGuiPopupFlags_MouseButtonRight), if you rely on the default parameter
//   and want to another another flag, you need to pass in the ImGuiPopupFlags_MouseButtonRight flag.
// - Multiple buttons currently cannot be combined/or-ed in those functions (we could allow it later).
public enum ImGuiPopupFlags_
{
    None = 0,
    MouseButtonLeft = 0,        // For BeginPopupContext*(): open on Left Mouse release. Guaranteed to always be == 0 (same as ImGuiMouseButton_Left)
    MouseButtonRight = 1,        // For BeginPopupContext*(): open on Right Mouse release. Guaranteed to always be == 1 (same as ImGuiMouseButton_Right)
    MouseButtonMiddle = 2,        // For BeginPopupContext*(): open on Middle Mouse release. Guaranteed to always be == 2 (same as ImGuiMouseButton_Middle)
    MouseButtonMask_ = 0x1F,
    MouseButtonDefault_ = 1,
    NoOpenOverExistingPopup = 1 << 5,   // For OpenPopup*(), BeginPopupContext*(): don't open if there's already a popup at the same level of the popup stack
    NoOpenOverItems = 1 << 6,   // For BeginPopupContextWindow(): don't return true when hovering items, only when hovering empty space
    AnyPopupId = 1 << 7,   // For IsPopupOpen(): ignore the ImGuiID parameter and test for any popup.
    AnyPopupLevel = 1 << 8,   // For IsPopupOpen(): search/test at any level of the popup stack (default test in the current level)
    AnyPopup = AnyPopupId | AnyPopupLevel
};

// Flags for ImGui::Selectable()
public enum ImGuiSelectableFlags
{
    None = 0,
    DontClosePopups = 1 << 0,   // Clicking this don't close parent popup window
    SpanAllColumns = 1 << 1,   // Selectable frame can span all columns (text will still fit in current column)
    AllowDoubleClick = 1 << 2,   // Generate press events on double clicks too
    Disabled = 1 << 3,   // Cannot be selected, display grayed out text
    AllowItemOverlap = 1 << 4    // (WIP) Hit testing to allow subsequent widgets to overlap this one
};

// Flags for ImGui::BeginCombo()
enum ImGuiComboFlags
{
    None = 0,
    PopupAlignLeft = 1 << 0,   // Align the popup toward the left by default
    HeightSmall = 1 << 1,   // Max ~4 items visible. Tip: If you want your combo popup to be a specific size you can use SetNextWindowSizeConstraints() prior to calling BeginCombo()
    HeightRegular = 1 << 2,   // Max ~8 items visible (default)
    HeightLarge = 1 << 3,   // Max ~20 items visible
    HeightLargest = 1 << 4,   // As many fitting items as possible
    NoArrowButton = 1 << 5,   // Display on the preview box without the square arrow button
    NoPreview = 1 << 6,   // Display only a square arrow button
    HeightMask_ = HeightSmall | HeightRegular | HeightLarge | HeightLargest
};

// Flags for ImGui::BeginTabBar()
public enum ImGuiTabBarFlags
{
    None = 0,
    Reorderable = 1 << 0,   // Allow manually dragging tabs to re-order them + New tabs are appended at the end of list
    AutoSelectNewTabs = 1 << 1,   // Automatically select new tabs when they appear
    TabListPopupButton = 1 << 2,   // Disable buttons to open the tab list popup
    NoCloseWithMiddleMouseButton = 1 << 3,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You can still repro this behavior on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
    NoTabListScrollingButtons = 1 << 4,   // Disable scrolling buttons (apply when fitting policy is ImGuiTabBarFlags_FittingPolicyScroll)
    NoTooltip = 1 << 5,   // Disable tooltips when hovering a tab
    FittingPolicyResizeDown = 1 << 6,   // Resize tabs when they don't fit
    FittingPolicyScroll = 1 << 7,   // Add scroll buttons when tabs don't fit
    FittingPolicyMask_    = FittingPolicyResizeDown | FittingPolicyScroll,
    FittingPolicyDefault_ = FittingPolicyResizeDown
};

// Flags for ImGui::BeginTabItem()
public enum ImGuiTabItemFlags
{
    None = 0,
    UnsavedDocument = 1 << 0,   // Display a dot next to the title + tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
    SetSelected = 1 << 1,   // Trigger flag to programmatically make the tab selected when calling BeginTabItem()
    NoCloseWithMiddleMouseButton = 1 << 2,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You can still repro this behavior on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
    NoPushId = 1 << 3,   // Don't call PushID(tab->ID)/PopID() on BeginTabItem()/EndTabItem()
    NoTooltip = 1 << 4,   // Disable tooltip for the given tab
    NoReorder = 1 << 5,   // Disable reordering this tab or having another tab cross over this tab
    Leading = 1 << 6,   // Enforce the tab position to the left of the tab bar (after the tab list popup button)
    Trailing = 1 << 7    // Enforce the tab position to the right of the tab bar (before the scrolling buttons)
};

// Flags for ImGui::BeginTable()
// [BETA API] API may evolve slightly! If you use this, please update to the next version when it comes out!
// - Important! Sizing policies have complex and subtle side effects, more so than you would expect.
//   Read comments/demos carefully + experiment with live demos to get acquainted with them.
// - The DEFAULT sizing policies are:
//    - Default to ImGuiTableFlags_SizingFixedFit    if ScrollX is on, or if host window has ImGuiWindowFlags_AlwaysAutoResize.
//    - Default to ImGuiTableFlags_SizingStretchSame if ScrollX is off.
// - When ScrollX is off:
//    - Table defaults to ImGuiTableFlags_SizingStretchSame -> all Columns defaults to ImGuiTableColumnFlags_WidthStretch with same weight.
//    - Columns sizing policy allowed: Stretch (default), Fixed/Auto.
//    - Fixed Columns will generally obtain their requested width (unless the table cannot fit them all).
//    - Stretch Columns will share the remaining width.
//    - Mixed Fixed/Stretch columns is possible but has various side-effects on resizing behaviors.
//      The typical use of mixing sizing policies is: any number of LEADING Fixed columns, followed by one or two TRAILING Stretch columns.
//      (this is because the visible order of columns have subtle but necessary effects on how they react to manual resizing).
// - When ScrollX is on:
//    - Table defaults to ImGuiTableFlags_SizingFixedFit -> all Columns defaults to ImGuiTableColumnFlags_WidthFixed
//    - Columns sizing policy allowed: Fixed/Auto mostly.
//    - Fixed Columns can be enlarged as needed. Table will show an horizontal scrollbar if needed.
//    - When using auto-resizing (non-resizable) fixed columns, querying the content width to use item right-alignment e.g. SetNextItemWidth(-FLT_MIN) doesn't make sense, would create a feedback loop.
//    - Using Stretch columns OFTEN DOES NOT MAKE SENSE if ScrollX is on, UNLESS you have specified a value for 'inner_width' in BeginTable().
//      If you specify a value for 'inner_width' then effectively the scrolling space is known and Stretch or mixed Fixed/Stretch columns become meaningful again.
// - Read on documentation at the top of imgui_tables.cpp for details.
public enum ImGuiTableFlags
{
    // Features
    None = 0,
    Resizable = 1 << 0,   // Enable resizing columns.
    Reorderable = 1 << 1,   // Enable reordering columns in header row (need calling TableSetupColumn() + TableHeadersRow() to display headers)
    Hideable = 1 << 2,   // Enable hiding/disabling columns in context menu.
    Sortable = 1 << 3,   // Enable sorting. Call TableGetSortSpecs() to obtain sort specs. Also see ImGuiTableFlags_SortMulti and ImGuiTableFlags_SortTristate.
    NoSavedSettings = 1 << 4,   // Disable persisting columns order, width and sort settings in the .ini file.
    ContextMenuInBody = 1 << 5,   // Right-click on columns body/contents will display table context menu. By default it is available in TableHeadersRow().
                                  // Decorations
    RowBg = 1 << 6,   // Set each RowBg color with ImGuiCol_TableRowBg or ImGuiCol_TableRowBgAlt (equivalent of calling TableSetBgColor with ImGuiTableBgFlags_RowBg0 on each row manually)
    BordersInnerH = 1 << 7,   // Draw horizontal borders between rows.
    BordersOuterH = 1 << 8,   // Draw horizontal borders at the top and bottom.
    BordersInnerV = 1 << 9,   // Draw vertical borders between columns.
    BordersOuterV = 1 << 10,  // Draw vertical borders on the left and right sides.
    BordersH     = BordersInnerH | BordersOuterH, // Draw horizontal borders.
    BordersV     = BordersInnerV | BordersOuterV, // Draw vertical borders.
    BordersInner = BordersInnerV | BordersInnerH, // Draw inner borders.
    BordersOuter = BordersOuterV | BordersOuterH, // Draw outer borders.
    Borders      = BordersInner  | BordersOuter,   // Draw all borders.
    NoBordersInBody = 1 << 11,  // [ALPHA] Disable vertical borders in columns Body (borders will always appears in Headers). -> May move to style
    NoBordersInBodyUntilResize = 1 << 12,  // [ALPHA] Disable vertical borders in columns Body until hovered for resize (borders will always appears in Headers). -> May move to style
                                           // Sizing Policy (read above for defaults)
    SizingFixedFit = 1 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching contents width.
    SizingFixedSame = 2 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching the maximum contents width of all columns. Implicitly enable ImGuiTableFlags_NoKeepColumnsVisible.
    SizingStretchProp = 3 << 13,  // Columns default to _WidthStretch with default weights proportional to each columns contents widths.
    SizingStretchSame = 4 << 13,  // Columns default to _WidthStretch with default weights all equal, unless overridden by TableSetupColumn().
                                  // Sizing Extra Options
    NoHostExtendX = 1 << 16,  // Make outer width auto-fit to columns, overriding outer_size.x value. Only available when ScrollX/ScrollY are disabled and Stretch columns are not used.
    NoHostExtendY = 1 << 17,  // Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit). Only available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.
    NoKeepColumnsVisible = 1 << 18,  // Disable keeping column always minimally visible when ScrollX is off and table gets too small. Not recommended if columns are resizable.
    PreciseWidths = 1 << 19,  // Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.
                              // Clipping
    NoClip = 1 << 20,  // Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with TableSetupScrollFreeze().
                       // Padding
    PadOuterX = 1 << 21,  // Default if BordersOuterV is on. Enable outer-most padding. Generally desirable if you have headers.
    NoPadOuterX = 1 << 22,  // Default if BordersOuterV is off. Disable outer-most padding.
    NoPadInnerX = 1 << 23,  // Disable inner padding between columns (double inner padding if BordersOuterV is on, single inner padding if BordersOuterV is off).
                            // Scrolling
    ScrollX = 1 << 24,  // Enable horizontal scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size. Changes default sizing policy. Because this create a child window, ScrollY is currently generally recommended when using ScrollX.
    ScrollY = 1 << 25,  // Enable vertical scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size.
                        // Sorting
    SortMulti = 1 << 26,  // Hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).
    SortTristate = 1 << 27,  // Allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).

};

// Flags for ImGui::TableSetupColumn()
public enum ImGuiTableColumnFlags
{
    // Input configuration flags
    None = 0,
    Disabled = 1 << 0,   // Overriding/master disable flag: hide column, won't show in context menu (unlike calling TableSetColumnEnabled() which manipulates the user accessible state)
    DefaultHide = 1 << 1,   // Default as a hidden/disabled column.
    DefaultSort = 1 << 2,   // Default as a sorting column.
    WidthStretch = 1 << 3,   // Column will stretch. Preferable with horizontal scrolling disabled (default if table sizing policy is _SizingStretchSame or _SizingStretchProp).
    WidthFixed = 1 << 4,   // Column will not stretch. Preferable with horizontal scrolling enabled (default if table sizing policy is _SizingFixedFit and table is resizable).
    NoResize = 1 << 5,   // Disable manual resizing.
    NoReorder = 1 << 6,   // Disable manual reordering this column, this will also prevent other columns from crossing over this column.
    NoHide = 1 << 7,   // Disable ability to hide/disable this column.
    NoClip = 1 << 8,   // Disable clipping for this column (all NoClip columns will render in a same draw command).
    NoSort = 1 << 9,   // Disable ability to sort on this field (even if ImGuiTableFlags_Sortable is set on the table).
    NoSortAscending = 1 << 10,  // Disable ability to sort in the ascending direction.
    NoSortDescending = 1 << 11,  // Disable ability to sort in the descending direction.
    NoHeaderLabel = 1 << 12,  // TableHeadersRow() will not submit label for this column. Convenient for some small columns. Name will still appear in context menu.
    NoHeaderWidth = 1 << 13,  // Disable header text width contribution to automatic column width.
    PreferSortAscending = 1 << 14,  // Make the initial sort direction Ascending when first sorting on this column (default).
    PreferSortDescending = 1 << 15,  // Make the initial sort direction Descending when first sorting on this column.
    IndentEnable = 1 << 16,  // Use current Indent value when entering cell (default for column 0).
    IndentDisable = 1 << 17,  // Ignore current Indent value when entering cell (default for columns > 0). Indentation changes _within_ the cell will still be honored.

    // Output status , read-only via TableGetColumnFlags()
    IsEnabled = 1 << 24,  // Status: is enabled == not hidden by user/api (referred to as "Hide" in _DefaultHide and _NoHide) flags.
    IsVisible = 1 << 25,  // Status: is visible == is enabled AND not clipped by scrolling.
    IsSorted = 1 << 26,  // Status: is currently part of the sort specs
    IsHovered = 1 << 27,  // Status: is hovered by mouse
};

// Flags for ImGui::TableNextRow()
public enum ImGuiTableRowFlags
{
    None = 0,
    Headers = 1 << 0    // Identify header row (set default background color + width of its contents accounted different for auto column width)
};

// Enum for ImGui::TableSetBgColor()
// Background colors are rendering in 3 layers:
//  - Layer 0: draw with RowBg0 color if set, otherwise draw with ColumnBg0 if set.
//  - Layer 1: draw with RowBg1 color if set, otherwise draw with ColumnBg1 if set.
//  - Layer 2: draw with CellBg color if set.
// The purpose of the two row/columns layers is to let you decide if a background color changes should override or blend with the existing color.
// When using ImGuiTableFlags_RowBg on the table, each row has the RowBg0 color automatically set for odd/even rows.
// If you set the color of RowBg0 target, your color will override the existing RowBg0 color.
// If you set the color of RowBg1 or ColumnBg1 target, your color will blend over the RowBg0 color.
public enum ImGuiTableBgTarget
{
    None = 0,
    RowBg0 = 1,        // Set row background color 0 (generally used for background, automatically set when ImGuiTableFlags_RowBg is used)
    RowBg1 = 2,        // Set row background color 1 (generally used for selection marking)
    CellBg = 3         // Set cell background color (top-most color)
};

// Flags for ImGui::IsWindowFocused()
public enum ImGuiFocusedFlags
{
    None = 0,
    ChildWindows = 1 << 0,   // Return true if any children of the window is focused
    RootWindow = 1 << 1,   // Test from root window (top most parent of the current hierarchy)
    AnyWindow = 1 << 2,   // Return true if any window is focused. Important: If you are trying to tell how to dispatch your low-level inputs, do NOT use this. Use 'io.WantCaptureMouse' instead! Please read the FAQ!
    NoPopupHierarchy = 1 << 3,   // Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
    DockHierarchy = 1 << 4,   // Consider docking hierarchy (treat dockspace host as parent of docked window) (when used with _ChildWindows or _RootWindow)
    RootAndChildWindows = RootWindow | ChildWindows
};

// Flags for ImGui::IsItemHovered(), ImGui::IsWindowHovered()
// Note: if you are trying to check whether your mouse should be dispatched to Dear ImGui or to your app, you should use 'io.WantCaptureMouse' instead! Please read the FAQ!
// Note: windows with the ImGuiWindowFlags_NoInputs flag are ignored by IsWindowHovered() calls.
public enum ImGuiHoveredFlags
{
    None = 0,        // Return true if directly over the item/window, not obstructed by another window, not obstructed by an active popup or modal blocking inputs under them.
    ChildWindows = 1 << 0,   // IsWindowHovered() only: Return true if any children of the window is hovered
    RootWindow = 1 << 1,   // IsWindowHovered() only: Test from root window (top most parent of the current hierarchy)
    AnyWindow = 1 << 2,   // IsWindowHovered() only: Return true if any window is hovered
    NoPopupHierarchy = 1 << 3,   // IsWindowHovered() only: Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
    DockHierarchy = 1 << 4,   // IsWindowHovered() only: Consider docking hierarchy (treat dockspace host as parent of docked window) (when used with _ChildWindows or _RootWindow)
    AllowWhenBlockedByPopup = 1 << 5,   // Return true even if a popup window is normally blocking access to this item/window
                                //ImGuiHoveredFlags_AllowWhenBlockedByModal     = 1 << 6,   // Return true even if a modal popup window is normally blocking access to this item/window. FIXME-TODO: Unavailable yet.
    AllowWhenBlockedByActiveItem = 1 << 7,   // Return true even if an active item is blocking access to this item/window. Useful for Drag and Drop patterns.
    AllowWhenOverlapped = 1 << 8,   // IsItemHovered() only: Return true even if the position is obstructed or overlapped by another window
    AllowWhenDisabled = 1 << 9,   // IsItemHovered() only: Return true even if the item is disabled
    // RectOnly = ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenOverlapped,
    // RootAndChildWindows = ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_ChildWindows
};

// Flags for ImGui::DockSpace(), shared/inherited by child nodes.
// (Some flags can be applied to individual nodes directly)
// FIXME-DOCK: Also see ImGuiDockNodeFlagsPrivate_ which may involve using the WIP and internal DockBuilder api.
public enum ImGuiDockNodeFlags
{
    None = 0,
    KeepAliveOnly = 1 << 0,   // Shared       // Don't display the dockspace node but keep it alive. Windows docked into this dockspace node won't be undocked.
                      //ImGuiDockNodeFlags_NoCentralNode              = 1 << 1,   // Shared       // Disable Central Node (the node which can stay empty)
    NoDockingInCentralNode = 1 << 2,   // Shared       // Disable docking inside the Central Node, which will be always kept empty.
    PassthruCentralNode = 1 << 3,   // Shared       // Enable passthru dockspace: 1) DockSpace() will render a ImGuiCol_WindowBg background covering everything excepted the Central Node when empty. Meaning the host window should probably use SetNextWindowBgAlpha(0.0f) prior to Begin() when using this. 2) When Central Node is empty: let inputs pass-through + won't display a DockingEmptyBg background. See demo for details.
    NoSplit = 1 << 4,   // Shared/Local // Disable splitting the node into smaller nodes. Useful e.g. when embedding dockspaces into a main root one (the root one may have splitting disabled to reduce confusion). Note: when turned off, existing splits will be preserved.
    NoResize = 1 << 5,   // Shared/Local // Disable resizing node using the splitter/separators. Useful with programmatically setup dockspaces.
    AutoHideTabBar = 1 << 6    // Shared/Local // Tab bar will automatically hide when there is a single window in the dock node.
};

// Flags for ImGui::BeginDragDropSource(), ImGui::AcceptDragDropPayload()
public enum ImGuiDragDropFlags
{
    None = 0,
    // BeginDragDropSource() flags
    SourceNoPreviewTooltip = 1 << 0,   // By default, a successful call to BeginDragDropSource opens a tooltip so you can display a preview or description of the source contents. This flag disable this behavior.
    SourceNoDisableHover = 1 << 1,   // By default, when dragging we clear data so that IsItemHovered() will return false, to avoid subsequent user code submitting tooltips. This flag disable this behavior so you can still call IsItemHovered() on the source item.
    SourceNoHoldToOpenOthers = 1 << 2,   // Disable the behavior that allows to open tree nodes and collapsing header by holding over them while dragging a source item.
    SourceAllowNullID = 1 << 3,   // Allow items such as Text(), Image() that have no unique identifier to be used as drag source, by manufacturing a temporary identifier based on their window-relative position. This is extremely unusual within the dear imgui ecosystem and so we made it explicit.
    SourceExtern = 1 << 4,   // External source (from outside of dear imgui), won't attempt to read current item/window info. Will always return true. Only one Extern source can be active simultaneously.
    SourceAutoExpirePayload = 1 << 5,   // Automatically expire the payload if the source cease to be submitted (otherwise payloads are persisting while being dragged)
                                        // AcceptDragDropPayload() flags
    AcceptBeforeDelivery = 1 << 10,  // AcceptDragDropPayload() will returns true even before the mouse button is released. You can then call IsDelivery() to test if the payload needs to be delivered.
    AcceptNoDrawDefaultRect = 1 << 11,  // Do not draw the default highlight rectangle when hovering over target.
    AcceptNoPreviewTooltip = 1 << 12,  // Request hiding the BeginDragDropSource tooltip from the BeginDragDropTarget site.
    // AcceptPeekOnly = ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect  // For peeking ahead and inspecting the payload before delivery.
};


// A primary data type
public enum ImGuiDataType
{
    S8,       // signed char / char (with sensible compilers)
    U8,       // unsigned char
    S16,      // short
    U16,      // unsigned short
    S32,      // int
    U32,      // unsigned int
    S64,      // long long / __int64
    U64,      // unsigned long long / unsigned __int64
    Float,    // float
    Double,   // double
    COUNT
};

// A cardinal direction
public enum ImGuiDir
{
    None = -1,
    Left = 0,
    Right = 1,
    Up = 2,
    Down = 3,
    COUNT
};

// A sorting direction
public enum ImGuiSortDirection
{
    None = 0,
    Ascending = 1,    // Ascending = 0->9, A->Z etc.
    Descending = 2     // Descending = 9->0, Z->A etc.
};

// User fill ImGuiIO.KeyMap[] array with indices into the ImGuiIO.KeysDown[512] array
public enum ImGuiKey
{
    Tab,
    LeftArrow,
    RightArrow,
    UpArrow,
    DownArrow,
    PageUp,
    PageDown,
    Home,
    End,
    Insert,
    Delete,
    Backspace,
    Space,
    Enter,
    Escape,
    KeyPadEnter,
    A,                 // for text edit CTRL+A: select all
    C,                 // for text edit CTRL+C: copy
    V,                 // for text edit CTRL+V: paste
    X,                 // for text edit CTRL+X: cut
    Y,                 // for text edit CTRL+Y: redo
    Z,                 // for text edit CTRL+Z: undo
    COUNT
};

// To test io.KeyMods (which is a combination of individual fields io.KeyCtrl, io.KeyShift, io.KeyAlt set by user/backend)
public enum ImGuiKeyModFlags
{
    None = 0,
    Ctrl = 1 << 0,
    Shift = 1 << 1,
    Alt = 1 << 2,
    Super = 1 << 3
};

// Gamepad/Keyboard navigation
// Keyboard: Set io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard to enable. NewFrame() will automatically fill io.NavInputs[] based on your io.KeysDown[] + io.KeyMap[] arrays.
// Gamepad:  Set io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad to enable. Backend: set ImGuiBackendFlags_HasGamepad and fill the io.NavInputs[] fields before calling NewFrame(). Note that io.NavInputs[] is cleared by EndFrame().
// Read instructions in imgui.cpp for more details. Download PNG/PSD at http://dearimgui.org/controls_sheets.
public enum ImGuiNavInput
{
    // Gamepad Mapping
    Activate,      // activate / open / toggle / tweak value       // e.g. Cross  (PS4), A (Xbox), A (Switch), Space (Keyboard)
    Cancel,        // cancel / close / exit                        // e.g. Circle (PS4), B (Xbox), B (Switch), Escape (Keyboard)
    Input,         // text input / on-screen keyboard              // e.g. Triang.(PS4), Y (Xbox), X (Switch), Return (Keyboard)
    Menu,          // tap: toggle menu / hold: focus, move, resize // e.g. Square (PS4), X (Xbox), Y (Switch), Alt (Keyboard)
    DpadLeft,      // move / tweak / resize window (w/ PadMenu)    // e.g. D-pad Left/Right/Up/Down (Gamepads), Arrow keys (Keyboard)
    DpadRight,     //
    DpadUp,        //
    DpadDown,      //
    LStickLeft,    // scroll / move window (w/ PadMenu)            // e.g. Left Analog Stick Left/Right/Up/Down
    LStickRight,   //
    LStickUp,      //
    LStickDown,    //
    FocusPrev,     // next window (w/ PadMenu)                     // e.g. L1 or L2 (PS4), LB or LT (Xbox), L or ZL (Switch)
    FocusNext,     // prev window (w/ PadMenu)                     // e.g. R1 or R2 (PS4), RB or RT (Xbox), R or ZL (Switch)
    TweakSlow,     // slower tweaks                                // e.g. L1 or L2 (PS4), LB or LT (Xbox), L or ZL (Switch)
    TweakFast     // faster tweaks                                // e.g. R1 or R2 (PS4), RB or RT (Xbox), R or ZL (Switch)
};

// Configuration flags stored in io.ConfigFlags. Set by user/application.
public enum ImGuiConfigFlags
{
    None = 0,
    NavEnableKeyboard = 1 << 0,   // Master keyboard navigation enable flag. NewFrame() will automatically fill io.NavInputs[] based on io.KeysDown[].
    NavEnableGamepad = 1 << 1,   // Master gamepad navigation enable flag. This is mostly to instruct your imgui backend to fill io.NavInputs[]. Backend also needs to set ImGuiBackendFlags_HasGamepad.
    NavEnableSetMousePos = 1 << 2,   // Instruct navigation to move the mouse cursor. May be useful on TV/console systems where moving a virtual mouse is awkward. Will update io.MousePos and set io.WantSetMousePos=true. If enabled you MUST honor io.WantSetMousePos requests in your backend, otherwise ImGui will react as if the mouse is jumping around back and forth.
    NavNoCaptureKeyboard = 1 << 3,   // Instruct navigation to not set the io.WantCaptureKeyboard flag when io.NavActive is set.
    NoMouse = 1 << 4,   // Instruct imgui to clear mouse position/buttons in NewFrame(). This allows ignoring the mouse information set by the backend.
    NoMouseCursorChange = 1 << 5,   // Instruct backend to not alter mouse cursor shape and visibility. Use if the backend cursor changes are interfering with yours and you don't want to use SetMouseCursor() to change mouse cursor. You may want to honor requests from imgui by reading GetMouseCursor() yourself instead.

    
    DockingEnable = 1 << 6,   // Docking enable flags.

    ViewportsEnable = 1 << 10,  // Viewport enable flags (require both ImGuiBackendFlags_PlatformHasViewports + ImGuiBackendFlags_RendererHasViewports set by the respective backends)
    DpiEnableScaleViewports = 1 << 14,  // [BETA: Don't use] FIXME-DPI: Reposition and resize imgui windows when the DpiScale of a viewport changed (mostly useful for the main viewport hosting other window). Note that resizing the main window itself is up to your application.
    DpiEnableScaleFonts = 1 << 15,  // [BETA: Don't use] FIXME-DPI: Request bitmap-scaled fonts to match DpiScale. This is a very low-quality workaround. The correct way to handle DPI is _currently_ to replace the atlas and/or fonts in the Platform_OnChangedViewport callback, but this is all early work in progress.

    IsSRGB = 1 << 20,  // Application is SRGB-aware.
    IsTouchScreen = 1 << 21   // Application is using a touch screen instead of a mouse.
};

// Backend capabilities flags stored in io.BackendFlags. Set by imgui_impl_xxx or custom backend.
public enum ImGuiBackendFlags
{
    None = 0,
    HasGamepad = 1 << 0,   // Backend Platform supports gamepad and currently has one connected.
    HasMouseCursors = 1 << 1,   // Backend Platform supports honoring GetMouseCursor() value to change the OS cursor shape.
    HasSetMousePos = 1 << 2,   // Backend Platform supports io.WantSetMousePos requests to reposition the OS mouse position (only used if ImGuiConfigFlags_NavEnableSetMousePos is set).
    RendererHasVtxOffset = 1 << 3,   // Backend Renderer supports ImDrawCmd::VtxOffset. This enables output of large meshes (64K+ vertices) while still using 16-bit indices.

    PlatformHasViewports = 1 << 10,  // Backend Platform supports multiple viewports.
    HasMouseHoveredViewport = 1 << 11,  // Backend Platform supports setting io.MouseHoveredViewport to the viewport directly under the mouse _IGNORING_ viewports with the ImGuiViewportFlags_NoInputs flag and _REGARDLESS_ of whether another viewport is focused and may be capturing the mouse. This information is _NOT EASY_ to provide correctly with most high-level engines! Don't set this without studying _carefully_ how the backends handle ImGuiViewportFlags_NoInputs!
    RendererHasViewports = 1 << 12   // Backend Renderer supports multiple viewports.
};

// Enumeration for PushStyleColor() / PopStyleColor()
public enum ImGuiCol
{
    Text,
    TextDisabled,
    WindowBg,              // Background of normal windows
    ChildBg,               // Background of child windows
    PopupBg,               // Background of popups, menus, tooltips windows
    Border,
    BorderShadow,
    FrameBg,               // Background of checkbox, radio button, plot, slider, text input
    FrameBgHovered,
    FrameBgActive,
    TitleBg,
    TitleBgActive,
    TitleBgCollapsed,
    MenuBarBg,
    ScrollbarBg,
    ScrollbarGrab,
    ScrollbarGrabHovered,
    ScrollbarGrabActive,
    CheckMark,
    SliderGrab,
    SliderGrabActive,
    Button,
    ButtonHovered,
    ButtonActive,
    Header,                // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
    HeaderHovered,
    HeaderActive,
    Separator,
    SeparatorHovered,
    SeparatorActive,
    ResizeGrip,
    ResizeGripHovered,
    ResizeGripActive,
    Tab,
    TabHovered,
    TabActive,
    TabUnfocused,
    TabUnfocusedActive,
    DockingPreview,        // Preview overlay color when about to docking something
    DockingEmptyBg,        // Background color for empty node (e.g. CentralNode with no window docked into it)
    PlotLines,
    PlotLinesHovered,
    PlotHistogram,
    PlotHistogramHovered,
    TableHeaderBg,         // Table header background
    TableBorderStrong,     // Table outer and header borders (prefer using Alpha=1.0 here)
    TableBorderLight,      // Table inner borders (prefer using Alpha=1.0 here)
    TableRowBg,            // Table row background (even rows)
    TableRowBgAlt,         // Table row background (odd rows)
    TextSelectedBg,
    DragDropTarget,
    NavHighlight,          // Gamepad/keyboard: current highlighted item
    NavWindowingHighlight, // Highlight window when using CTRL+TAB
    NavWindowingDimBg,     // Darken/colorize entire screen behind the CTRL+TAB window list, when active
    ModalWindowDimBg,      // Darken/colorize entire screen behind a modal window, when one is active
    COUNT
};

// Enumeration for PushStyleVar() / PopStyleVar() to temporarily modify the ImGuiStyle structure.
// - The enum only refers to fields of ImGuiStyle which makes sense to be pushed/popped inside UI code.
//   During initialization or between frames, feel free to just poke into ImGuiStyle directly.
// - Tip: Use your programming IDE navigation facilities on the names in the _second column_ below to find the actual members and their description.
//   In Visual Studio IDE: CTRL+comma ("Edit.NavigateTo") can follow symbols in comments, whereas CTRL+F12 ("Edit.GoToImplementation") cannot.
//   With Visual Assist installed: ALT+G ("VAssistX.GoToImplementation") can also follow symbols in comments.
// - When changing this enum, you need to update the associated internal table GStyleVarInfo[] accordingly. This is where we link enum values to members offset/type.
public enum ImGuiStyleVar
{
    // Enum name --------------------- // Member in ImGuiStyle structure (see ImGuiStyle for descriptions)
   Alpha,               // float     Alpha
   DisabledAlpha,       // float     DisabledAlpha
   WindowPadding,       // ImVec2    WindowPadding
   WindowRounding,      // float     WindowRounding
   WindowBorderSize,    // float     WindowBorderSize
   WindowMinSize,       // ImVec2    WindowMinSize
   WindowTitleAlign,    // ImVec2    WindowTitleAlign
   ChildRounding,       // float     ChildRounding
   ChildBorderSize,     // float     ChildBorderSize
   PopupRounding,       // float     PopupRounding
   PopupBorderSize,     // float     PopupBorderSize
   FramePadding,        // ImVec2    FramePadding
   FrameRounding,       // float     FrameRounding
   FrameBorderSize,     // float     FrameBorderSize
   ItemSpacing,         // ImVec2    ItemSpacing
   ItemInnerSpacing,    // ImVec2    ItemInnerSpacing
   IndentSpacing,       // float     IndentSpacing
   CellPadding,         // ImVec2    CellPadding
   ScrollbarSize,       // float     ScrollbarSize
   ScrollbarRounding,   // float     ScrollbarRounding
   GrabMinSize,         // float     GrabMinSize
   GrabRounding,        // float     GrabRounding
   TabRounding,         // float     TabRounding
   ButtonTextAlign,     // ImVec2    ButtonTextAlign
   SelectableTextAlign, // ImVec2    SelectableTextAlign
   COUNT
};

// Flags for InvisibleButton() [extended in imgui_internal.h]
public enum ImGuiButtonFlags
{
    None = 0,
    MouseButtonLeft = 1 << 0,   // React on left mouse button (default)
    MouseButtonRight = 1 << 1,   // React on right mouse button
    MouseButtonMiddle = 1 << 2,   // React on center mouse button
};

// Flags for ColorEdit3() / ColorEdit4() / ColorPicker3() / ColorPicker4() / ColorButton()
public enum ImGuiColorEditFlags
{
    None = 0,
    NoAlpha = 1 << 1,   //              // ColorEdit, ColorPicker, ColorButton: ignore Alpha component (will only read 3 components from the input pointer).
    NoPicker = 1 << 2,   //              // ColorEdit: disable picker when clicking on color square.
    NoOptions = 1 << 3,   //              // ColorEdit: disable toggling options menu when right-clicking on inputs/small preview.
    NoSmallPreview = 1 << 4,   //              // ColorEdit, ColorPicker: disable color square preview next to the inputs. (e.g. to show only the inputs)
    NoInputs = 1 << 5,   //              // ColorEdit, ColorPicker: disable inputs sliders/text widgets (e.g. to show only the small preview color square).
    NoTooltip = 1 << 6,   //              // ColorEdit, ColorPicker, ColorButton: disable tooltip when hovering the preview.
    NoLabel = 1 << 7,   //              // ColorEdit, ColorPicker: disable display of inline text label (the label is still forwarded to the tooltip and picker).
    NoSidePreview = 1 << 8,   //              // ColorPicker: disable bigger color preview on right side of the picker, use small color square preview instead.
    NoDragDrop = 1 << 9,   //              // ColorEdit: disable drag and drop target. ColorButton: disable drag and drop source.
    NoBorder = 1 << 10,  //              // ColorButton: disable border (which is enforced by default)

    AlphaBar = 1 << 16,  //              // ColorEdit, ColorPicker: show vertical alpha bar/gradient in picker.
    AlphaPreview = 1 << 17,  //              // ColorEdit, ColorPicker, ColorButton: display preview as a transparent color over a checkerboard, instead of opaque.
    AlphaPreviewHalf = 1 << 18,  //              // ColorEdit, ColorPicker, ColorButton: display half opaque / half checkerboard, instead of opaque.
    HDR = 1 << 19,  //              // (WIP) ColorEdit: Currently only disable 0.0f..1.0f limits in RGBA edition (note: you probably want to use ImGuiColorEditFlags_Float flag as well).
    DisplayRGB = 1 << 20,  // [Display]    // ColorEdit: override _display_ type among RGB/HSV/Hex. ColorPicker: select any combination using one or more of RGB/HSV/Hex.
    DisplayHSV = 1 << 21,  // [Display]    // "
    DisplayHex = 1 << 22,  // [Display]    // "
    Uint8 = 1 << 23,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0..255.
    Float = 1 << 24,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0.0f..1.0f floats instead of 0..255 integers. No round-trip of value via integers.
    PickerHueBar = 1 << 25,  // [Picker]     // ColorPicker: bar for Hue, rectangle for Sat/Value.
    PickerHueWheel = 1 << 26,  // [Picker]     // ColorPicker: wheel for Hue, triangle for Sat/Value.
    InputRGB = 1 << 27,  // [Input]      // ColorEdit, ColorPicker: input and output data in RGB format.
    InputHSV = 1 << 28,  // [Input]      // ColorEdit, ColorPicker: input and output data in HSV format.

    DefaultOptions = Uint8 | DisplayRGB | InputRGB | PickerHueBar,
};

// Flags for DragFloat(), DragInt(), SliderFloat(), SliderInt() etc.
// We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.
public enum ImGuiSliderFlags
{
    None = 0,
    AlwaysClamp = 1 << 4,       // Clamp value to min/max bounds when input manually with CTRL+Click. By default CTRL+Click allows going out of bounds.
    Logarithmic = 1 << 5,       // Make the widget logarithmic (linear otherwise). Consider using ImGuiSliderFlags_NoRoundToFormat with this if using a format-string with small amount of digits.
    NoRoundToFormat = 1 << 6,       // Disable rounding underlying value to match precision of the display format string (e.g. %.3f values are rounded to those 3 digits)
    NoInput = 1 << 7,       // Disable CTRL+Click or Enter key allowing to input text directly into the widget
    InvalidMask_ = 0x7000000F    // [Internal] We treat using those bits as being potentially a 'float power' argument from the previous API that has got miscast to this enum, and will trigger an assert if needed.
};

// Identify a mouse button.
// Those values are guaranteed to be stable and we frequently use 0/1 directly. Named enums provided for convenience.
public enum ImGuiMouseButton
{
    Left = 0,
    Right = 1,
    Middle = 2,
    COUNT = 5
};

// Enumeration for GetMouseCursor()
// User code may request backend to display given cursor by calling SetMouseCursor(), which is why we have some cursors that are marked unused here
public enum ImGuiMouseCursor
{
    None = -1,
    Arrow = 0,
    TextInput,         // When hovering over InputText, etc.
    ResizeAll,         // (Unused by Dear ImGui functions)
    ResizeNS,          // When hovering over an horizontal border
    ResizeEW,          // When hovering over a vertical border or a column
    ResizeNESW,        // When hovering over the bottom-left corner of a window
    ResizeNWSE,        // When hovering over the bottom-right corner of a window
    Hand,              // (Unused by Dear ImGui functions. Use for e.g. hyperlinks)
    NotAllowed,        // When hovering something with disallowed interaction. Usually a crossed circle.
    COUNT
};

// Enumeration for ImGui::SetWindow***(), SetNextWindow***(), SetNextItem***() functions
// Represent a condition.
// Important: Treat as a regular enum! Do NOT combine multiple values using binary operators! All the functions above treat 0 as a shortcut to ImGuiCond_Always.
public enum ImGuiCond
{
    None = 0,        // No condition (always set the variable), same as _Always
    Always = 1 << 0,   // No condition (always set the variable)
    Once = 1 << 1,   // Set the variable once per runtime session (only the first call will succeed)
    FirstUseEver = 1 << 2,   // Set the variable if the object/window has no persistently saved data (no entry in .ini file)
    Appearing = 1 << 3    // Set the variable if the object/window is appearing after being hidden/inactive (or the first time)
};
