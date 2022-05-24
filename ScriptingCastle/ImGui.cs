/*
using System;
using System.Numerics;
using System.Runtime.CompilerServices;

using ImVec2  = System.Numerics.Vector2;
using ImVec4  = System.Numerics.Vector4;
using ImGuiID = System.UInt32;
using ImU32   = System.UInt32;
using ImGuiViewportFlags = System.Int32;
using ImTextureID = System.UInt64;

public struct iVec2
{ 
    public int x, y;
    iVec2(int x, int y) { this.x = x; this.y = y; }
    public static iVec2 operator +(iVec2 a, iVec2 b) {
        return new iVec2(a.x + b.x, a.y + b.y);
    }
    public static iVec2 operator -(iVec2 a, iVec2 b) {
        return new iVec2(a.x - b.x, a.y - b.y);
    }
}
public struct ImGuiViewport
{
    ImGuiID ID;                    // Unique identifier for the viewport
    ImGuiViewportFlags Flags;      // See ImGuiViewportFlags_
    ImVec2 Pos;                    // Main Area: Position of the viewport (Dear ImGui coordinates are the same as OS desktop/native coordinates)
    ImVec2 Size;                   // Main Area: Size of the viewport.
    ImVec2 WorkPos;                // Work Area: Position of the viewport minus task bars, menus bars, status bars (>= Pos)
    ImVec2 WorkSize;               // Work Area: Size of the viewport minus task bars, menu bars, status bars (<= Size)
    float DpiScale;                // 1.0f = 96 DPI = No extra scale.
    ImGuiID ParentViewportId;      // (Advanced) 0: no parent. Instruct the platform backend to setup a parent/child relationship between platform windows.
    UIntPtr DrawData;              // The ImDrawData corresponding to this viewport. Valid after Render() and until the next call to NewFrame().

    // Our design separate the Renderer and Platform backends to facilitate combining default backends with each others.
    // When our create your own backend for a custom engine, it is possible that both Renderer and Platform will be handled
    // by the same system and you may not need to use all the UserData/Handle fields.
    // The library never uses those fields, they are merely storage to facilitate backend implementation.
    UIntPtr RendererUserData;       // void* to hold custom data structure for the renderer (e.g. swap chain, framebuffers etc.). generally set by your Renderer_CreateWindow function.
    UIntPtr PlatformUserData;       // void* to hold custom data structure for the OS / platform (e.g. windowing info, render context). generally set by your Platform_CreateWindow function.
    UIntPtr PlatformHandle;         // void* for FindViewportByPlatformHandle(). (e.g. suggested to use natural platform handle such as HWND, GLFWWindow*, SDL_Window*)
    UIntPtr PlatformHandleRaw;      // void* to hold lower-level, platform-native window handle (e.g. the HWND) when using an abstraction layer like GLFW or SDL (where PlatformHandle would be a SDL_Window*)

    bool PlatformRequestMove;    // Platform window requested move (e.g. window was moved by the OS / host window manager, authoritative position will be OS window position)
    bool PlatformRequestResize;  // Platform window requested resize (e.g. window was resized by the OS / host window manager, authoritative size will be OS window size)
    bool PlatformRequestClose;   // Platform window requested closure (e.g. window was moved by the OS / host window manager, e.g. pressing ALT-F4)

    // Helpers
    public ImVec2 GetCenter()     { return new ImVec2(Pos.X + Size.X * 0.5f, Pos.Y + Size.Y * 0.5f); }
    public ImVec2 GetWorkCenter() { return new ImVec2(WorkPos.X + WorkSize.X * 0.5f, WorkPos.Y + WorkSize.Y * 0.5f); }
}

public static class ImGui
{
    [MethodImpl(MethodImplOptions.InternalCall)]
    // fixme: p_ open is always true for now
    public static extern bool Begin(string name, bool p_open = true, ImGuiWindowFlags flags = 0);
    
    [MethodImpl(MethodImplOptions.InternalCall)]
    public static extern void End();

    // Child Windows
    // - Use child windows to begin into a self-contained independent scrolling/clipping regions within a host window. Child windows can embed their own child.
    // - For each independent axis of 'size': ==0.0f: use remaining host window size / >0.0f: fixed size / <0.0f: use remaining window size minus abs(size) / Each axis can use a different mode, e.g. ImVec2(0,400).
    // - BeginChild() returns false to indicate the window is collapsed or fully clipped, so you may early out and omit submitting anything to the window.
    //   Always call a matching EndChild() for each BeginChild() call, regardless of its return value.
    //   [Important: due to legacy reason, this is inconsistent with most other functions such as BeginMenu/EndMenu,
    //   BeginPopup/EndPopup, etc. where the EndXXX call should only be called if the corresponding BeginXXX function
    //   returned true. Begin and BeginChild are the only odd ones out. Will be fixed in a future update.]
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginChild(string str_id, ImVec2 size, bool border = false, ImGuiWindowFlags flags = 0);
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginChild(ImGuiID id, ImVec2 size, bool border = false, ImGuiWindowFlags flags = 0);
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndChild();
    
    // Windows Utilities
    // - 'current window' = the window we are appending into while inside a Begin()/End() block. 'next window' = next window we will Begin() into.
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool IsWindowAppearing();
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool IsWindowCollapsed();
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool IsWindowFocused(ImGuiFocusedFlags flags = 0); // is current window focused? or its root/child, depending on flags. see flags for options.
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool IsWindowHovered(ImGuiHoveredFlags flags = 0); // is current window hovered (and typically: not blocked by a popup/modal)? see flags for options. NB: If you are trying to check whether your mouse should be dispatched to imgui or to your app, you should use the 'io.WantCaptureMouse' boolean for that! Please read the FAQ!
    // ImDrawList*   GetWindowDrawList();															     // get draw list associated to the current window, to append your own drawing primitives
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern float  GetWindowDpiScale ();       // get DPI scale currently associated to the current window's viewport.
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2 GetWindowPos      ();       // get current window position in screen space (useful if you want to do your own drawing via the DrawList API)
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2 GetWindowSize     ();       // get current window size
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern float  GetWindowWidth    ();       // get current window width (shortcut for GetWindowSize().x)
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern float  GetWindowHeight   ();       // get current window height (shortcut for GetWindowSize().y)
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImGuiViewport GetWindowViewport(); // g

    // Window manipulation
    // - Prefer using SetNextXXX functions (before Begin) rather that SetXXX functions (after Begin).
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetNextWindowPos        (ImVec2 pos, ImGuiCond cond, ImVec2 pivot); // set next window position. call before Begin(). use pivot=(0.5f,0.5f) to center on given point, etc.
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetNextWindowSize       (ImVec2 size, ImGuiCond cond = 0);                  // set next window size. set axis to 0.0f to force an auto-fit on this axis. call before Begin()
    // public static extern void SetNextWindowSizeConstraints(ImVec2 size_min, ImVec2 size_max, ImGuiSizeCallback custom_callback , void* custom_callback_data = NULL); // set next window size limits. use -1,-1 on either X/Y axis to preserve the current size. Sizes will be rounded down. Use callback to apply non-trivial programmatic constraints.
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetNextWindowContentSize(ImVec2 size);                               // set next window content size (~ scrollable client area, which enforce the range of scrollbars). Not including window decorations (title bar, menu bar, etc.) nor WindowPadding. set an axis to 0.0f to leave it automatic. call before Begin()
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetNextWindowCollapsed  (bool collapsed, ImGuiCond cond = 0);          // set next window collapsed state. call before Begin()
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetNextWindowViewport   (ImGuiID viewport_id);                          // set next window viewport
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetWindowPos            (ImVec2 pos, ImGuiCond cond = 0);                        // (not recommended) set current window position - call within Begin()/End(). prefer using SetNextWindowPos(), as this may incur tearing and side-effects.
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetWindowSize           (ImVec2 size, ImGuiCond cond = 0);                      // (not recommended) set current window size - call within Begin()/End(). set to ImVec2(0, 0) to force an auto-fit. prefer using SetNextWindowSize(), as this may incur tearing and minor side-effects.
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetWindowCollapsed      (bool collapsed, ImGuiCond cond = 0);              // (not recommended) set current window collapsed state. prefer using SetNextWindowCollapsed().
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetWindowPos            (string name, ImVec2 pos, ImGuiCond cond = 0);      // set named window position.
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetWindowSize           (string name, ImVec2 size, ImGuiCond cond = 0);    // set named window size. set axis to 0.0f to force an auto-fit on this axis.
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetWindowCollapsed      (string name, bool collapsed, ImGuiCond cond = 0);   // set named window collapsed state
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetWindowFocus          (string name);                                           // set named window to be focused / top-most. use NULL to remove focus.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetNextWindowFocus      ();                                                // set next window to be focused / top-most. call before Begin()
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetNextWindowBgAlpha    (float alpha);                                   // set next window background color alpha. helper to easily override the Alpha component of ImGuiCol_WindowBg/ChildBg/PopupBg. you may also use ImGuiWindowFlags_NoBackground.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetWindowFontScale      (float scale);                                     // [OBSOLETE] set font scale. Adjust IO.FontGlobalScale if you want to scale all windows. This is an old API! For correct scaling, prefer to reload font + rebuild ImFontAtlas + call style.ScaleAllSizes().
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetWindowFocus          ();                                                    // (not recommended) set current window to be focused / top-most. prefer using SetNextWindowFocus().

    // Content region
    // - Retrieve available space from a given point. GetContentRegionAvail() is frequently useful.
    // - Those functions are bound to be redesigned (they are confusing, incomplete and the Min/Max return values are in local window coordinates which increases confusion)
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2  GetContentRegionAvail    ();                                 // == GetContentRegionMax() - GetCursorPos()
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2  GetContentRegionMax      ();                                 // current content boundaries (typically window boundaries including scrolling, or current column boundaries), in windows coordinates
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2  GetWindowContentRegionMin();                                 // content boundaries min for the full window (roughly (0,0)-Scroll), in window coordinates
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2  GetWindowContentRegionMax();                                 // content boundaries max for the full window (roughly (0,0)+Size-Scroll) where Size can be override with SetNextWindowContentSize(), in window coordinates

    // Windows Scrolling
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern float GetScrollX       ();                                                  // get scrolling amount [0 .. GetScrollMaxX()]
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern float GetScrollY       ();                                                  // get scrolling amount [0 .. GetScrollMaxY()]
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void  SetScrollX       (float scroll_x);                                     // set scrolling amount [0 .. GetScrollMaxX()]
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void  SetScrollY       (float scroll_y);                                     // set scrolling amount [0 .. GetScrollMaxY()]
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern float GetScrollMaxX    ();                                               // get maximum scrolling amount ~~ ContentSize.x - WindowSize.x - DecorationsSize.x
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern float GetScrollMaxY    ();                                               // get maximum scrolling amount ~~ ContentSize.y - WindowSize.y - DecorationsSize.y
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void  SetScrollHereX   (float center_x_ratio = 0.5f);                    // adjust scrolling amount to make current cursor position visible. center_x_ratio=0.0: left, 0.5: center, 1.0: right. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void  SetScrollHereY   (float center_y_ratio = 0.5f);                    // adjust scrolling amount to make current cursor position visible. center_y_ratio=0.0: top, 0.5: center, 1.0: bottom. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void  SetScrollFromPosX(float local_x, float center_x_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void  SetScrollFromPosY(float local_y, float center_y_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.

    // Parameters stacks (shared)
    // todo: change font from csharp
    // [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PushFont(ImFont* font);                           // use NULL as a shortcut to push default font
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PopFont	              ();
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PushStyleColor         (ImGuiCol idx, ImU32 col);             // modify a style color. always use this if you modify the style after NewFrame().
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PushStyleColor         (ImGuiCol idx, ImVec4 col);
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PopStyleColor          (int count = 1);
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PushStyleVar           (ImGuiStyleVar idx, float val);          // modify a style float variable. always use this if you modify the style after NewFrame().
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PushStyleVar           (ImGuiStyleVar idx, ImVec2 val);         // modify a style ImVec2 variable. always use this if you modify the style after NewFrame().
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PopStyleVar            (int count = 1);
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PushAllowKeyboardFocus (bool allow_keyboard_focus);   // == tab stop enable. Allow focusing using TAB/Shift-TAB, enabled by default but you can disable it for certain widgets
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PopAllowKeyboardFocus  ();
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PushButtonRepeat       (bool repeat);                       // in 'repeat' mode, Button*() functions return repeated true in a typematic manner (using io.KeyRepeatDelay/io.KeyRepeatRate setting). Note that you can call IsItemActive() after any Button() to tell if the button is held in the current frame.
    [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PopButtonRepeat        ();

    // Parameters stacks(current window)
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void  PushItemWidth    (float item_width);                                // push width of items for common large "item+label" widgets. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side).
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void  PopItemWidth     ();
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void  SetNextItemWidth (float item_width);                             // set width of the _next_ common large "item+label" widget. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side)
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern float CalcItemWidth    ();                                                // width of item given pushed settings and current cursor position. NOT necessarily the width of last item unlike most 'Item' functions.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void  PushTextWrapPos  (float wrap_local_pos_x = 0.0f);                 // push word-wrapping position for Text*() commands. < 0.0f: no wrapping; 0.0f: wrap to end of window (or column); > 0.0f: wrap at 'wrap_pos_x' position in window local space
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void  PopTextWrapPos   ();

    // Style read access
    // - Use the style editor (ShowStyleEditor() function) to interactively see what the colors are)
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImFont*       GetFont();                                                      // get current font
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern float   GetFontSize           ();                                                  // get current font size (= height in pixels) of current font with current scale applied
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2  GetFontTexUvWhitePixel();                                       // get UV coordinate for a while pixel, useful to draw custom shapes via the ImDrawList API
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern ImU32   GetColorU32           (ImGuiCol idx, float alpha_mul = 1.0f);              // retrieve given style color with style alpha applied and optional extra alpha multiplier, packed as a 32-bit value suitable for ImDrawList
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern ImU32   GetColorU32           (ImVec4 col);                                 // retrieve given color with style alpha applied, packed as a 32-bit value suitable for ImDrawList
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern ImU32   GetColorU32           (ImU32 col);                                         // retrieve given color with style alpha applied, packed as a 32-bit value suitable for ImDrawList
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec4  GetStyleColorVec4     (ImGuiCol idx);                                // retrieve style color as stored in ImGuiStyle structure. use to feed back into PushStyleColor(), otherwise use GetColorU32() to get style color with style alpha baked in.

    // Cursor / Layout
    // - By "cursor" we mean the current output position.
    // - The typical widget behavior is to output themselves at the current cursor position, then move the cursor one line down.
    // - You can call SameLine() between widgets to undo the last carriage return and output at the right of the preceding widget.
    // - Attention! We currently have inconsistencies between window-local and absolute positions we will aim to fix with future API:
    //    Window-local coordinates:   SameLine(), GetCursorPos(), SetCursorPos(), GetCursorStartPos(), GetContentRegionMax(), GetWindowContentRegion*(), PushTextWrapPos()
    //    Absolute coordinate:        GetCursorScreenPos(), SetCursorScreenPos(), all ImDrawList:: functions.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void    Separator					  ();                                                        // separator, generally horizontal. inside a menu bar or in horizontal layout mode, this becomes a vertical separator.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void    SameLine 					  (float offset_from_start_x = 0.0f, float spacing = -1.0f);  // call between widgets or groups to layout them horizontally. X position given in window coordinates.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void    NewLine                     ();                                                          // undo a SameLine() or force a new line when in an horizontal-layout context.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void    Spacing                     ();                                                          // add vertical spacing.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void    Dummy					      (ImVec2 size);                                                 // add a dummy item of given size. unlike InvisibleButton(), Dummy() won't take the mouse click or be navigable into.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void    Indent					  (float indent_w = 0.0f);                                      // move content position toward the right, by indent_w, or style.IndentSpacing if indent_w <= 0
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void    Unindent				      (float indent_w = 0.0f);                                    // move content position back to the left, by indent_w, or style.IndentSpacing if indent_w <= 0
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void    BeginGroup                  ();                                                       // lock horizontal starting position
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void    EndGroup					  ();                                                         // unlock horizontal starting position + capture the whole group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.)
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2  GetCursorPos	              ();                                                     // cursor position in window coordinates (relative to window position)
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern float   GetCursorPosX				  ();                                                    //   (some functions are using window-relative coordinates, such as: GetCursorPos, GetCursorStartPos, GetContentRegionMax, GetWindowContentRegion* etc.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern float   GetCursorPosY				  ();                                                    //    other functions such as GetCursorScreenPos or everything in ImDrawList::
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void    SetCursorPos	  			  (ImVec2 local_pos);                                     //    are using the main, absolute coordinate system.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void    SetCursorPosX				  (float local_x);                                       //    GetWindowPos() + GetCursorPos() == GetCursorScreenPos() etc.)
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void    SetCursorPosY				  (float local_y);                                       //
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2  GetCursorStartPos	          ();                                                // initial cursor position in window coordinates
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2  GetCursorScreenPos		  ();                                               // cursor position in absolute coordinates (useful to work with ImDrawList API). generally top-left == GetMainViewport()->Pos == (0,0) in single viewport mode, and bottom-right == GetMainViewport()->Pos+Size == io.DisplaySize in single-viewport mode.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void    SetCursorScreenPos		  (ImVec2 pos);                          			  // cursor position in absolute coordinates
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void    AlignTextToFramePadding     ();                                          // vertically align upcoming text baseline to FramePadding.y so that it will align properly to regularly framed items (call if you have text on a line before a framed item)
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern float   GetTextLineHeight           ();                                                // ~ FontSize
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern float   GetTextLineHeightWithSpacing();                                     // ~ FontSize + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of text)
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern float   GetFrameHeight			  ();                                                   // ~ FontSize + style.FramePadding.y * 2
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern float   GetFrameHeightWithSpacing   ();                                        // ~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)

    // ID stack/scopes
    // Read the FAQ (docs/FAQ.md or http://dearimgui.org/faq) for more details about how ID are handled in dear imgui.
    // - Those questions are answered and impacted by understanding of the ID stack system:
    //   - "Q: Why is my widget not reacting when I click on it?"
    //   - "Q: How can I have widgets with an empty label?"
    //   - "Q: How can I have multiple widgets with the same label?"
    // - Short version: ID are hashes of the entire ID stack. If you are creating widgets in a loop you most likely
    //   want to push a unique identifier (e.g. object pointer, loop index) to uniquely differentiate them.
    // - You can also use the "Label##foobar" syntax within widget label to distinguish them from each others.
    // - In this header file we use the "label"/"name" terminology to denote a string that will be displayed + used as an ID,
    //   whereas "str_id" denote a string that is only used as an ID and not normally displayed.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void PushID(string str_id);                                     // push string into the ID stack (will hash string).
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void PushID(string str_id_begin, string str_id_end);       // push string into the ID stack (will hash string).
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PushID(const void* ptr_id);                                     // push pointer into the ID stack (will hash pointer).
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void PushID(int int_id);                                             // push integer into the ID stack (will hash integer).
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void PopID();                                                        // pop from the ID stack.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern ImGuiID GetID(string str_id);                                      // calculate unique ID (hash of whole ID stack + given parameter). e.g. if you want to query into ImGuiStorage yourself
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern ImGuiID GetID(string str_id_begin, string str_id_end);
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImGuiID GetID(const void* ptr_id);

    // Widgets: Text
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void TextUnformatted(string text, string text_end = "")   ; // raw text without formatting. Roughly equivalent to Text("%s", text) but: A) doesn't require null terminated string if 'text_end' is specified, B) it's faster, no memory copy is done, no buffer size limits, recommended for long chunks of text.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void Text           (string fmt, ...)                            ; // formatted text
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void TextV          (string fmt, va_list args)                   ;
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void TextColored    (ImVec4& col, const char* fmt, ...)         ; // shortcut for PushStyleColor(ImGuiCol_Text, col); Text(fmt, ...); PopStyleColor();
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void TextColoredV   (ImVec4& col, const char* fmt, va_list args);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void TextDisabled   (string fmt, ...)                            ; // shortcut for PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]); Text(fmt, ...); PopStyleColor();
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void TextDisabledV  (string fmt, va_list args)                   ;
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void TextWrapped    (string fmt, ...)                            ; // shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void TextWrappedV   (string fmt, va_list args)                   ;
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void LabelText      (string label, string fmt, ...)              ; // display text+label aligned the same way as value+label widgets
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void LabelTextV     (string label, string fmt, va_list args)     ;
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void BulletText     (string fmt, ...)                            ; // shortcut for Bullet()+Text()
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void BulletTextV    (string fmt, va_list args)                   ;

    // Widgets: Main
    // - Most widgets return true when the value has been changed or when pressed/selected
    // - You may also use one of the many IsItemXXX functions (e.g. IsItemActive, IsItemHovered, etc.) to query widget state.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool Button         (string label, ImVec2 size);   // button
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool SmallButton    (string label);                                 // button with FramePadding=(0,0) to easily embed within text
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InvisibleButton(string str_id, ImVec2 size, ImGuiButtonFlags flags = 0); // flexible button behavior without the visuals, frequently useful to build custom behaviors using the public api (along with IsItemActive, IsItemHovered, etc.)
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool ArrowButton    (string str_id, ImGuiDir dir);                  // square button with an arrow shape
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void Image          (ImTextureID user_texture_id, ImVec2 size, ImVec2 uv0, ImVec2 uv1, ImVec4 tint_col, ImVec4 border_col);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool ImageButton    (ImTextureID user_texture_id, ImVec2 size, ImVec2 uv0, ImVec2 uv1, int frame_padding = -1, ImVec4 bg_col, ImVec4 tint_col);    // <0 frame_padding uses default frame padding settings. 0 for no padding
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool Checkbox       (string label, bool* v);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool CheckboxFlags  (string label, int* flags, int flags_value);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool CheckboxFlags  (string label, unsigned int* flags, unsigned int flags_value);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool RadioButton    (string label, bool active);                    // use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; }
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool RadioButton    (string label, int* v, int v_button);           // shortcut to handle the above pattern when value is an integer
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void ProgressBar    (float fraction, ImVec2 size_arg, string overlay = ""); // size_arg = ImVec2(-FLT_MIN, 0)
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void Bullet();                                                       // draw a small circle + keep the cursor on the same line. advance cursor x position by GetTreeNodeToLabelSpacing(), same distance that TreeNode() uses

    // Widgets: Combo Box
    // - The BeginCombo()/EndCombo() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() items.
    // - The old Combo() api are helpers over BeginCombo()/EndCombo() which are kept available for convenience purpose. This is analogous to how ListBox are created.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginCombo(string label, string preview_value, ImGuiComboFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndCombo(); // only call EndCombo() if BeginCombo() returns true!
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool Combo(string label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool Combo(string label, int* current_item, string items_separated_by_zeros, int popup_max_height_in_items = -1);      // Separate items with \0 within a string, end item-list with \0\0. e.g. "One\0Two\0Three\0"
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool Combo(string label, int* current_item, bool (* items_getter) (void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items = -1);

    // Widgets: Drag Sliders
    // - CTRL+Click on any drag box to turn them into an input box. Manually input values aren't clamped by default and can go off-bounds. Use ImGuiSliderFlags_AlwaysClamp to always clamp.
    // - For all the Float2/Float3/Float4/Int2/Int3/Int4 versions of every functions, note that a 'float v[X]' function argument is the same as 'float* v', the array syntax is just a way to document the number of elements that are expected to be accessible. You can pass address of your first element out of a contiguous set, e.g. &myvector.x
    // - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
    // - Format string may also be set to NULL or use the default format ("%f" or "%d").
    // - Speed are per-pixel of mouse movement (v_speed=0.2f: mouse needs to move by 5 pixels to increase value by 1). For gamepad/keyboard navigation, minimum speed is Max(v_speed, minimum_step_at_given_precision).
    // - Use v_min < v_max to clamp edits to given limits. Note that CTRL+Click manual input can override those limits if ImGuiSliderFlags_AlwaysClamp is not used.
    // - Use v_max = FLT_MAX / INT_MAX etc to avoid clamping to a maximum, same with v_min = -FLT_MAX / INT_MIN to avoid clamping to a minimum.
    // - We use the same sets of flags for DragXXX() and SliderXXX() functions as the features are the same and it makes it easier to swap them.
    // - Legacy: Pre-1.78 there are DragXXX() function signatures that takes a final `float power=1.0f' argument instead of the `ImGuiSliderFlags flags=0' argument.
    //   If you get a warning converting a float to ImGuiSliderFlags, read https://github.com/ocornut/imgui/issues/3361
	
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool DragFloat      (string label, float* v  , float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, string format = "%.3f", ImGuiSliderFlags flags = 0);     // If v_min >= v_max we have no bound
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool DragFloat2     (string label, float v[2], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, string format = "%.3f", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool DragFloat3     (string label, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, string format = "%.3f", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool DragFloat4     (string label, float v[4], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, string format = "%.3f", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool DragInt        (string label, int* v    , float v_speed = 1.0f, int v_min = 0, int v_max = 0, string format = "%d", ImGuiSliderFlags flags = 0);  // If v_min >= v_max we have no bound
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool DragInt2       (string label, int v[2]  , float v_speed = 1.0f, int v_min = 0, int v_max = 0, string format = "%d", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool DragInt3       (string label, int v[3]  , float v_speed = 1.0f, int v_min = 0, int v_max = 0, string format = "%d", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool DragInt4       (string label, int v[4]  , float v_speed = 1.0f, int v_min = 0, int v_max = 0, string format = "%d", ImGuiSliderFlags flags = 0);
	
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool DragFloatRange2(string label, float* v_current_min, float* v_current_max, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", const char* format_max = NULL, ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool DragIntRange2  (string label, int* v_current_min, int* v_current_max, float v_speed = 1.0f, int v_min = 0, int v_max = 0, string format = "%d", string format_max = NULL, ImGuiSliderFlags flags = 0);
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool DragScalar     (string label, ImGuiDataType data_type, void* p_data, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, string format = NULL, ImGuiSliderFlags flags = 0);
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool DragScalarN    (string label, ImGuiDataType data_type, void* p_data, int components, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, string format = NULL, ImGuiSliderFlags flags = 0);

    // Widgets: Regular Sliders
    // - CTRL+Click on any slider to turn them into an input box. Manually input values aren't clamped by default and can go off-bounds. Use ImGuiSliderFlags_AlwaysClamp to always clamp.
    // - Adjust format string to decorate the value with a prefix, a suffix, or adapt the editing and display precision e.g. "%.3f" -> 1.234; "%5.2f secs" -> 01.23 secs; "Biscuit: %.0f" -> Biscuit: 1; etc.
    // - Format string may also be set to NULL or use the default format ("%f" or "%d").
    // - Legacy: Pre-1.78 there are SliderXXX() function signatures that takes a final `float power=1.0f' argument instead of the `ImGuiSliderFlags flags=0' argument.
    //   If you get a warning converting a float to ImGuiSliderFlags, read https://github.com/ocornut/imgui/issues/3361
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool SliderFloat  (string label, float* v  , float v_min, float v_max, string format = "%.3f", ImGuiSliderFlags flags = 0);     // adjust format to decorate the value with a prefix or a suffix for in-slider labels or unit display.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool SliderFloat2 (string label, float v[2], float v_min, float v_max, string format = "%.3f", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool SliderFloat3 (string label, float v[3], float v_min, float v_max, string format = "%.3f", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool SliderFloat4 (string label, float v[4], float v_min, float v_max, string format = "%.3f", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool SliderAngle  (string label, float* v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const char* format = "%.0f deg", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool SliderInt    (string label, int* v  , int v_min, int v_max, string format = "%d", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool SliderInt2   (string label, int v[2], int v_min, int v_max, string format = "%d", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool SliderInt3   (string label, int v[3], int v_min, int v_max, string format = "%d", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool SliderInt4   (string label, int v[4], int v_min, int v_max, string format = "%d", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool SliderScalar (string label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool SliderScalarN(string label, ImGuiDataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool VSliderFloat (string label, ImVec2 size, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool VSliderInt   (string label, ImVec2 size, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool VSliderScalar(string label, ImVec2 size, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, ImGuiSliderFlags flags = 0);

    // Widgets: Input with Keyboard
    // - If you want to use InputText() with std::string or any custom dynamic string type, see misc/cpp/imgui_stdlib.h and comments in imgui_demo.cpp.
    // - Most of the ImGuiInputTextFlags flags are only useful for InputText() and not for InputFloatX, InputIntX, InputDouble etc.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InputText         (string label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InputTextMultiline(string label, char* buf, size_t buf_size, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InputTextWithHint (string label, string hint, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InputFloat        (string label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InputFloat2       (string label, float v[2], string format = "%.3f", ImGuiInputTextFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InputFloat3       (string label, float v[3], string format = "%.3f", ImGuiInputTextFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InputFloat4       (string label, float v[4], string format = "%.3f", ImGuiInputTextFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InputInt          (string label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InputInt2         (string label, int v[2], ImGuiInputTextFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InputInt3         (string label, int v[3], ImGuiInputTextFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InputInt4         (string label, int v[4], ImGuiInputTextFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InputDouble       (string label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0);
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InputScalar       (string label, ImGuiDataType data_type, void* p_data, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, ImGuiInputTextFlags flags = 0);
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool InputScalarN      (string label, ImGuiDataType data_type, void* p_data, int components, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, ImGuiInputTextFlags flags = 0);

    // Widgets: Color Editor/Picker (tip: the ColorEdit* functions have a little color square that can be left-clicked to open a picker, and right-clicked to open an option menu.)
    // - Note that in C++ a 'float v[X]' function argument is the _same_ as 'float* v', the array syntax is just a way to document the number of elements that are expected to be accessible.
    // - You can pass the address of a first float element out of a contiguous structure, e.g. &myvector.x
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool ColorEdit3  (string label, float col[3], ImGuiColorEditFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool ColorEdit4  (string label, float col[4], ImGuiColorEditFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool ColorPicker3(string label, float col[3], ImGuiColorEditFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool ColorPicker4(string label, float col[4], ImGuiColorEditFlags flags = 0, const float* ref_col = NULL);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool ColorButton (string desc_id, const ImVec4& col, ImGuiColorEditFlags flags = 0, ImVec2 size = ImVec2(0, 0)); // display a color square/button, hover for details, return true when pressed.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetColorEditOptions(ImGuiColorEditFlags flags);                     // initialize current options (generally on application startup) if you want to select a default format, picker type, etc. User will be able to change many settings, unless you pass the _NoOptions flag to your calls.

    // Widgets: Trees
    // - TreeNode functions return true when the node is open, in which case you need to also call TreePop() when you are finished displaying the tree node contents.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool TreeNode   (string label);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool TreeNode   (string str_id, string fmt, ...);   // helper variation to easily decorelate the id from the displayed string. Read the FAQ about why and how to use ID. to align arbitrary text at the same level as a TreeNode() you can use Bullet().
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool TreeNode   (string ptr_id, string fmt, ...);   // "
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool TreeNodeV  (string str_id, string fmt, va_list args);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool TreeNodeV  (string ptr_id, string fmt, va_list args);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool TreeNodeEx (string label, ImGuiTreeNodeFlags flags = 0);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool TreeNodeEx (string str_id, ImGuiTreeNodeFlags flags, string fmt, ...) ;
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool TreeNodeEx (string ptr_id, ImGuiTreeNodeFlags flags, string fmt, ...) ;
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool TreeNodeExV(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args) IM_FMTLIST(3);
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool TreeNodeExV(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args) IM_FMTLIST(3);
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void TreePush   (string str_id);                                       // ~ Indent()+PushId(). Already called by TreeNode() when returning true, but you can call TreePush/TreePop yourself if desired.
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern void TreePush   (const void* ptr_id = NULL);                                // "
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void TreePop();                                                          // ~ Unindent()+PopId()
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern float GetTreeNodeToLabelSpacing();                                        // horizontal distance preceding label when using TreeNode*() or Bullet() == (g.FontSize + style.FramePadding.x*2) for a regular unframed TreeNode
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool CollapsingHeader(string label, ImGuiTreeNodeFlags flags = 0);  // if returning 'true' the header is open. doesn't indent nor push on ID stack. user doesn't have to call TreePop().
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool CollapsingHeader(string label, bool* p_visible, ImGuiTreeNodeFlags flags = 0); // when 'p_visible != NULL': if '*p_visible==true' display an additional small close button on upper right of the header which will set the bool to false when clicked, if '*p_visible==false' don't display the header.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetNextItemOpen (bool is_open, ImGuiCond cond = 0);                  // set next TreeNode/CollapsingHeader open state.

    // Widgets: Selectables
    // - A selectable highlights when hovered, and can display another color when selected.
    // - Neighbors selectable extend their highlight bounds in order to leave no gap between them. This is so a series of selected Selectable appear contiguous.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool Selectable(string label, bool selected = false, ImGuiSelectableFlags flags, ImVec2 size); // "bool selected" carry the selection state (read-only). Selectable() is clicked is returns true so you can modify your selection state. size.x==0.0: use remaining width, size.x>0.0: specify width. size.y==0.0: use label height, size.y>0.0: specify height
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool Selectable(string label, bool* p_selected,      ImGuiSelectableFlags flags, ImVec2 size); // "bool* p_selected" point to the selection state (read-write), as a convenient helper.

    // - This is essentially a thin wrapper to using BeginChild/EndChild with some stylistic changes.
    // - The BeginListBox()/EndListBox() api allows you to manage your contents and selection state however you want it, by creating e.g. Selectable() or any items.
    // - The simplified/old ListBox() api are helpers over BeginListBox()/EndListBox() which are kept available for convenience purpose. This is analoguous to how Combos are created.
    // - Choose frame width:   size.x > 0.0f: custom  /  size.x < 0.0f or -FLT_MIN: right-align   /  size.x = 0.0f (default): use current ItemWidth
    // - Choose frame height:  size.y > 0.0f: custom  /  size.y < 0.0f or -FLT_MIN: bottom-align  /  size.y = 0.0f (default): arbitrary default height which can fit ~7 items
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginListBox(string label, const ImVec2& size = ImVec2(0, 0)); // open a framed scrolling region
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndListBox();                                                       // only call EndListBox() if BeginListBox() returned true!
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool ListBox(string label, int* current_item, const char* const items[], int items_count, int height_in_items = -1);
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool ListBox(string label, int* current_item, bool (* items_getter) (void* data, int idx, const char** out_text), void* data, int items_count, int height_in_items = -1);
	//      
    //      // Widgets: Data Plotting
    //      // - Consider using ImPlot (https://github.com/epezent/implot) which is much better!
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PlotLines    (string label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float));
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PlotLines    (string label, float (* values_getter) (void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0));
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PlotHistogram(string label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float));
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PlotHistogram(string label, float (* values_getter) (void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0));
	//      
    //      // Widgets: Value() Helpers.
    //      // - Those are merely shortcut to calling Text() with a format string. Output single value in "name: value" format (tip: freely declare more in your code to handle your types. you can add functions to the ImGui namespace)
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Value(string prefix, bool b);
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Value(string prefix, int v);
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Value(string prefix, unsigned int v);
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Value(string prefix, float v, const char* float_format = NULL);
	//      
    //      // Widgets: Menus
    //      // - Use BeginMenuBar() on a window ImGuiWindowFlags_MenuBar to append to its menu bar.
    //      // - Use BeginMainMenuBar() to create a menu bar at the top of the screen and append to it.
    //      // - Use BeginMenu() to create a menu. You can call BeginMenu() multiple time with the same identifier to append more items to it.
    //      // - Not that MenuItem() keyboardshortcuts are displayed as a convenience but _not processed_ by Dear ImGui at the moment.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginMenuBar();                                                     // append to menu-bar of current window (requires ImGuiWindowFlags_MenuBar flag set on parent window).
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndMenuBar();                                                       // only call EndMenuBar() if BeginMenuBar() returns true!
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginMainMenuBar();                                                 // create and append to a full screen menu-bar.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndMainMenuBar();                                                   // only call EndMainMenuBar() if BeginMainMenuBar() returns true!
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginMenu(string label, bool enabled = true);                  // create a sub-menu entry. only call EndMenu() if this returns true!
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndMenu();                                                          // only call EndMenu() if BeginMenu() returns true!
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool MenuItem (string label, string shortcut = NULL, bool selected = false, bool enabled = true);  // return true when activated.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool MenuItem (string label, string shortcut, bool* p_selected, bool enabled = true);              // return true when activated + toggle (*p_selected) if p_selected != NULL
	//      
    //      // Tooltips
    //      // - Tooltip are windows following the mouse. They do not take focus away.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void BeginTooltip();                                                     // begin/append a tooltip window. to create full-featured tooltip (with any kind of items).
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndTooltip();
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetTooltip (string fmt, ...) IM_FMTARGS(1);                     // set a text-only tooltip, typically use with ImGui::IsItemHovered(). override any previous call to SetTooltip().
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetTooltipV(string fmt, va_list args) IM_FMTLIST(1);
	//      
    //      // Popups, Modals
    //      //  - They block normal mouse hovering detection (and therefore most mouse interactions) behind them.
    //      //  - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
    //      //  - Their visibility state (~bool) is held internally instead of being held by the programmer as we are used to with regular Begin*() calls.
    //      //  - The 3 properties above are related: we need to retain popup visibility state in the library because popups may be closed as any time.
    //      //  - You can bypass the hovering restriction by using ImGuiHoveredFlags_AllowWhenBlockedByPopup when calling IsItemHovered() or IsWindowHovered().
    //      //  - IMPORTANT: Popup identifiers are relative to the current ID stack, so OpenPopup and BeginPopup generally needs to be at the same level of the stack.
    //      //    This is sometimes leading to confusing mistakes. May rework this in the future.
	//      
    //      // Popups: begin/end functions
    //      //  - BeginPopup(): query popup state, if open start appending into the window. Call EndPopup() afterwards. ImGuiWindowFlags are forwarded to the window.
    //      //  - BeginPopupModal(): block every interactions behind the window, cannot be closed by user, add a dimming background, has a title bar.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginPopup     (string str_id, ImGuiWindowFlags flags = 0);                         // return true if the popup is open, and you can start outputting to it.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginPopupModal(string name, bool* p_open = NULL, ImGuiWindowFlags flags = 0); // return true if the modal is open, and you can start outputting to it.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndPopup();                                                                         // only call EndPopup() if BeginPopupXXX() returns true!
	//      
    //      // Popups: open/close functions
    //      //  - OpenPopup(): set popup state to open. ImGuiPopupFlags are available for opening options.
    //      //  - If not modal: they can be closed by clicking anywhere outside them, or by pressing ESCAPE.
    //      //  - CloseCurrentPopup(): use inside the BeginPopup()/EndPopup() scope to close manually.
    //      //  - CloseCurrentPopup() is called by default by Selectable()/MenuItem() when activated (FIXME: need some options).
    //      //  - Use ImGuiPopupFlags_NoOpenOverExistingPopup to avoid opening a popup if there's already one at the same level. This is equivalent to e.g. testing for !IsAnyPopupOpen() prior to OpenPopup().
    //      //  - Use IsWindowAppearing() after BeginPopup() to tell if a window just opened.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void OpenPopup(const char* str_id, ImGuiPopupFlags popup_flags = 0);                     // call to mark popup as open (don't call every frame!).
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void OpenPopup(ImGuiID id, ImGuiPopupFlags popup_flags = 0);                             // id overload to facilitate calling from nested stacks
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void OpenPopupOnItemClick(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);   // helper to open popup when clicked on last item. Default to ImGuiPopupFlags_MouseButtonRight == 1. (note: actually triggers on the mouse _released_ event to be consistent with popup behaviors)
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void CloseCurrentPopup();                                                                // manually close the popup we have begin-ed into.
	//      
    //      // Popups: open+begin combined functions helpers
    //      //  - Helpers to do OpenPopup+BeginPopup where the Open action is triggered by e.g. hovering an item and right-clicking.
    //      //  - They are convenient to easily create context menus, hence the name.
    //      //  - IMPORTANT: Notice that BeginPopupContextXXX takes ImGuiPopupFlags just like OpenPopup() and unlike BeginPopup(). For full consistency, we may add ImGuiWindowFlags to the BeginPopupContextXXX functions in the future.
    //      //  - IMPORTANT: we exceptionally default their flags to 1 (== ImGuiPopupFlags_MouseButtonRight) for backward compatibility with older API taking 'int mouse_button = 1' parameter, so if you add other flags remember to re-add the ImGuiPopupFlags_MouseButtonRight.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginPopupContextItem(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);  // open+begin popup when clicked on last item. Use str_id==NULL to associate the popup to previous item. If you want to use that on a non-interactive item such as Text() you need to pass in an explicit ID here. read comments in .cpp!
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginPopupContextWindow(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);// open+begin popup when clicked on current window.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginPopupContextVoid(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);  // open+begin popup when clicked in void (where there are no windows).
	//      
    //      // Popups: query functions
    //      //  - IsPopupOpen(): return true if the popup is open at the current BeginPopup() level of the popup stack.
    //      //  - IsPopupOpen() with ImGuiPopupFlags_AnyPopupId: return true if any popup is open at the current BeginPopup() level of the popup stack.
    //      //  - IsPopupOpen() with ImGuiPopupFlags_AnyPopupId + ImGuiPopupFlags_AnyPopupLevel: return true if any popup is open.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool IsPopupOpen(const char* str_id, ImGuiPopupFlags flags = 0);                         // return true if the popup is open.
	//      
    //      // Tables
    //      // [BETA API] API may evolve slightly! If you use this, please update to the next version when it comes out!
    //      // - Full-featured replacement for old Columns API.
    //      // - See Demo->Tables for demo code.
    //      // - See top of imgui_tables.cpp for general commentary.
    //      // - See ImGuiTableFlags_ and ImGuiTableColumnFlags_ enums for a description of available flags.
    //      // The typical call flow is:
    //      // - 1. Call BeginTable().
    //      // - 2. Optionally call TableSetupColumn() to submit column name/flags/defaults.
    //      // - 3. Optionally call TableSetupScrollFreeze() to request scroll freezing of columns/rows.
    //      // - 4. Optionally call TableHeadersRow() to submit a header row. Names are pulled from TableSetupColumn() data.
    //      // - 5. Populate contents:
    //      //    - In most situations you can use TableNextRow() + TableSetColumnIndex(N) to start appending into a column.
    //      //    - If you are using tables as a sort of grid, where every columns is holding the same type of contents,
    //      //      you may prefer using TableNextColumn() instead of TableNextRow() + TableSetColumnIndex().
    //      //      TableNextColumn() will automatically wrap-around into the next row if needed.
    //      //    - IMPORTANT: Comparatively to the old Columns() API, we need to call TableNextColumn() for the first column!
    //      //    - Summary of possible call flow:
    //      //        --------------------------------------------------------------------------------------------------------
    //      //        TableNextRow() -> TableSetColumnIndex(0) -> Text("Hello 0") -> TableSetColumnIndex(1) -> Text("Hello 1")  // OK
    //      //        TableNextRow() -> TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK
    //      //                          TableNextColumn()      -> Text("Hello 0") -> TableNextColumn()      -> Text("Hello 1")  // OK: TableNextColumn() automatically gets to next row!
    //      //        TableNextRow()                           -> Text("Hello 0")                                               // Not OK! Missing TableSetColumnIndex() or TableNextColumn()! Text will not appear!
    //      //        --------------------------------------------------------------------------------------------------------
    //      // - 5. Call EndTable()
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginTable(const char* str_id, int column, ImGuiTableFlags flags = 0, const ImVec2& outer_size = ImVec2(0.0f, 0.0f), float inner_width = 0.0f);
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndTable();                                 // only call EndTable() if BeginTable() returns true!
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void TableNextRow(ImGuiTableRowFlags row_flags = 0, float min_row_height = 0.0f); // append into the first cell of a new row.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool TableNextColumn();                          // append into the next column (or first column of next row if currently in last column). Return true when column is visible.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool TableSetColumnIndex(int column_n);          // append into the specified column. Return true when column is visible.
	//      
    //      // Tables: Headers & Columns declaration
    //      // - Use TableSetupColumn() to specify label, resizing policy, default width/weight, id, various other flags etc.
    //      // - Use TableHeadersRow() to create a header row and automatically submit a TableHeader() for each column.
    //      //   Headers are required to perform: reordering, sorting, and opening the context menu.
    //      //   The context menu can also be made available in columns body using ImGuiTableFlags_ContextMenuInBody.
    //      // - You may manually submit headers using TableNextRow() + TableHeader() calls, but this is only useful in
    //      //   some advanced use cases (e.g. adding custom widgets in header row).
    //      // - Use TableSetupScrollFreeze() to lock columns/rows so they stay visible when scrolled.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void TableSetupColumn(const char* label, ImGuiTableColumnFlags flags = 0, float init_width_or_weight = 0.0f, ImGuiID user_id = 0);
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void TableSetupScrollFreeze(int cols, int rows); // lock columns/rows so they stay visible when scrolled.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void TableHeadersRow();                          // submit all headers cells based on data provided to TableSetupColumn() + submit context menu
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void TableHeader(const char* label);             // submit one header cell manually (rarely used)
	//      
    //      // Tables: Sorting
    //      // - Call TableGetSortSpecs() to retrieve latest sort specs for the table. NULL when not sorting.
    //      // - When 'SpecsDirty == true' you should sort your data. It will be true when sorting specs have changed
    //      //   since last call, or the first time. Make sure to set 'SpecsDirty = false' after sorting, else you may
    //      //   wastefully sort your data every frame!
    //      // - Lifetime: don't hold on this pointer over multiple frames or past any subsequent call to BeginTable().
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImGuiTableSortSpecs*  TableGetSortSpecs();                        // get latest sort specs for the table (NULL if not sorting).
	//      
    //      // Tables: Miscellaneous functions
    //      // - Functions args 'int column_n' treat the default value of -1 as the same as passing the current column index.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern int TableGetColumnCount();                      // return number of columns (value passed to BeginTable)
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern int TableGetColumnIndex();                      // return current column index.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern int TableGetRowIndex();                         // return current row index.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern const char* TableGetColumnName(int column_n = -1);      // return "" if column didn't have a name declared by TableSetupColumn(). Pass -1 to use current column.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImGuiTableColumnFlags TableGetColumnFlags(int column_n = -1);     // return column flags so you can query their Enabled/Visible/Sorted/Hovered status flags. Pass -1 to use current column.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void TableSetColumnEnabled(int column_n, bool v);// change user accessible enabled/disabled state of a column. Set to false to hide the column. User can use the context menu to change this themselves (right-click in headers, or right-click in columns body with ImGuiTableFlags_ContextMenuInBody)
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void TableSetBgColor(ImGuiTableBgTarget target, ImU32 color, int column_n = -1);  // change the color of a cell, row, or column. See ImGuiTableBgTarget_ flags for details.
	//      
    //      // Legacy Columns API (prefer using Tables!)
    //      // - You can also use SameLine(pos_x) to mimic simplified columns.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void Columns(int count = 1, const char* id = NULL, bool border = true);
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void NextColumn();                                                       // next column, defaults to current row or next row if the current row is finished
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern int GetColumnIndex();                                                   // get current column index
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern float GetColumnWidth(int column_index = -1);                              // get column width (in pixels). pass -1 to use current column
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetColumnWidth(int column_index, float width);                      // set column width (in pixels). pass -1 to use current column
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern float GetColumnOffset(int column_index = -1);                             // get position of column line (in pixels, from the left side of the contents region). pass -1 to use current column, otherwise 0..GetColumnsCount() inclusive. column 0 is typically 0.0f
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetColumnOffset(int column_index, float offset_x);                  // set position of column line (in pixels, from the left side of the contents region). pass -1 to use current column
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern int GetColumnsCount();
	//      
    //      // Tab Bars, Tabs
    //      // Note: Tabs are automatically created by the docking system. Use this to create tab bars/tabs yourself without docking being involved.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginTabBar(const char* str_id, ImGuiTabBarFlags flags = 0);        // create and append into a TabBar
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndTabBar();                                                        // only call EndTabBar() if BeginTabBar() returns true!
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginTabItem(const char* label, bool* p_open = NULL, ImGuiTabItemFlags flags = 0); // create a Tab. Returns true if the Tab is selected.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndTabItem();                                                       // only call EndTabItem() if BeginTabItem() returns true!
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool TabItemButton(const char* label, ImGuiTabItemFlags flags = 0);      // create a Tab behaving like a button. return true when clicked. cannot be selected in the tab bar.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetTabItemClosed(const char* tab_or_docked_window_label);           // notify TabBar or Docking system of a closed tab/window ahead (useful to reduce visual flicker on reorderable tab bars). For tab-bar: call after BeginTabBar() and before Tab submissions. Otherwise call with a window name.
	//      
    //      // Docking
    //      // [BETA API] Enable with io.ConfigFlags |= ImGuiConfigFlags_DockingEnable.
    //      // Note: You can use most Docking facilities without calling any API. You DO NOT need to call DockSpace() to use Docking!
    //      // - Drag from window title bar or their tab to dock/undock. Hold SHIFT to disable docking.
    //      // - Drag from window menu button (upper-left button) to undock an entire node (all windows).
    //      // About dockspaces:
    //      // - Use DockSpace() to create an explicit dock node _within_ an existing window. See Docking demo for details.
    //      // - Use DockSpaceOverViewport() to create an explicit dock node covering the screen or a specific viewport.
    //      //   This is often used with ImGuiDockNodeFlags_PassthruCentralNode.
    //      // - Important: Dockspaces need to be submitted _before_ any window they can host. Submit it early in your frame!
    //      // - Important: Dockspaces need to be kept alive if hidden, otherwise windows docked into it will be undocked.
    //      //   e.g. if you have multiple tabs with a dockspace inside each tab: submit the non-visible dockspaces with ImGuiDockNodeFlags_KeepAliveOnly.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImGuiID       DockSpace(ImGuiID id, const ImVec2& size = ImVec2(0, 0), ImGuiDockNodeFlags flags = 0, const ImGuiWindowClass* window_class = NULL);
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImGuiID       DockSpaceOverViewport(const ImGuiViewport* viewport = NULL, ImGuiDockNodeFlags flags = 0, const ImGuiWindowClass* window_class = NULL);
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetNextWindowDockID(ImGuiID dock_id, ImGuiCond cond = 0);           // set next window dock id
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetNextWindowClass(const ImGuiWindowClass* window_class);           // set next window class (control docking compatibility + provide hints to platform backend via custom viewport flags and platform parent/child relationship)
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImGuiID       GetWindowDockID();
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool IsWindowDocked();                                                   // is current window docked into another window?
	//      
    //      // Logging/Capture
    //      // - All text output from the interface can be captured into tty/file/clipboard. By default, tree nodes are automatically opened during logging.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void LogToTTY(int auto_open_depth = -1);                                 // start logging to tty (stdout)
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void LogToFile(int auto_open_depth = -1, const char* filename = NULL);   // start logging to file
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void LogToClipboard(int auto_open_depth = -1);                           // start logging to OS clipboard
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void LogFinish();                                                        // stop logging (close file, etc.)
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void LogButtons();                                                       // helper to display buttons for logging to tty/file/clipboard
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void LogText(const char* fmt, ...) IM_FMTARGS(1);                        // pass text data straight to log (without being displayed)
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void LogTextV(const char* fmt, va_list args) IM_FMTLIST(1);
	//      
    //      // Drag and Drop
    //      // - On source items, call BeginDragDropSource(), if it returns true also call SetDragDropPayload() + EndDragDropSource().
    //      // - On target candidates, call BeginDragDropTarget(), if it returns true also call AcceptDragDropPayload() + EndDragDropTarget().
    //      // - If you stop calling BeginDragDropSource() the payload is preserved however it won't have a preview tooltip (we currently display a fallback "..." tooltip, see #1725)
    //      // - An item can be both drag source and drop target.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginDragDropSource(ImGuiDragDropFlags flags = 0);                                      // call after submitting an item which may be dragged. when this return true, you can call SetDragDropPayload() + EndDragDropSource()
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool SetDragDropPayload(const char* type, const void* data, size_t sz, ImGuiCond cond = 0);  // type is a user defined string of maximum 32 characters. Strings starting with '_' are reserved for dear imgui internal types. Data is copied and held by imgui.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndDragDropSource();                                                                    // only call EndDragDropSource() if BeginDragDropSource() returns true!
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginDragDropTarget();                                                          // call after submitting an item that may receive a payload. If this returns true, you can call AcceptDragDropPayload() + EndDragDropTarget()
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern const ImGuiPayload* AcceptDragDropPayload(const char* type, ImGuiDragDropFlags flags = 0);          // accept contents of a given type. If ImGuiDragDropFlags_AcceptBeforeDelivery is set you can peek into the payload before the mouse button is released.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndDragDropTarget();                                                            // only call EndDragDropTarget() if BeginDragDropTarget() returns true!
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern const ImGuiPayload* GetDragDropPayload();                                                           // peek directly into the current payload from anywhere. may return NULL. use ImGuiPayload::IsDataType() to test for the payload type.
	//      
    //      // Disabling [BETA API]
    //      // - Disable all user interactions and dim items visuals (applying style.DisabledAlpha over current colors)
    //      // - Those can be nested but it cannot be used to enable an already disabled section (a single BeginDisabled(true) in the stack is enough to keep everything disabled)
    //      // - BeginDisabled(false) essentially does nothing useful but is provided to facilitate use of boolean expressions. If you can avoid calling BeginDisabled(False)/EndDisabled() best to avoid it.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void BeginDisabled(bool disabled = true);
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndDisabled();
	//      
    //      // Clipping
    //      // - Mouse hovering is affected by ImGui::PushClipRect() calls, unlike direct calls to ImDrawList::PushClipRect() which are render only.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PushClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect);
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void PopClipRect();
	//      
    //      // Focus, Activation
    //      // - Prefer using "SetItemDefaultFocus()" over "if (IsWindowAppearing()) SetScrollHereY()" when applicable to signify "this is the default item"
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetItemDefaultFocus();                                              // make last item the default focused item of a window.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetKeyboardFocusHere(int offset = 0);                               // focus keyboard on the next widget. Use positive 'offset' to access sub components of a multiple component widget. Use -1 to access previous widget.
	//      
	 // Item/Widgets Utilities and Query Functions
	 // - Most of the functions are referring to the previous Item that has been submitted.
	 // - See Demo Window under "Widgets->Querying Status" for an interactive visualization of most of those functions.
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsItemHovered              (ImGuiHoveredFlags flags = 0);                         // is the last item hovered? (and usable, aka not blocked by a popup, etc.). See ImGuiHoveredFlags for more options.
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsItemActive               ();                                                     // is the last item active? (e.g. button being held, text field being edited. This will continuously return true while holding mouse button on an item. Items that don't interact will always return false)
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsItemFocused              ();                                                    // is the last item focused for keyboard/gamepad navigation?
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsItemClicked              (ImGuiMouseButton mouse_button = 0);                   // is the last item hovered and mouse clicked on? (**)  == IsMouseClicked(mouse_button) && IsItemHovered()Important. (**) this it NOT equivalent to the behavior of e.g. Button(). Read comments in function definition.
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsItemVisible              ();                                                    // is the last item visible? (items may be out of sight because of clipping/scrolling)
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsItemEdited               ();                                                     // did the last item modify its underlying value this frame? or was pressed? This is generally the same as the "bool" return value of many widgets.
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsItemActivated            ();                                                  // was the last item just made active (item was previously inactive).
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsItemDeactivated          ();                                                // was the last item just made inactive (item was previously active). Useful for Undo/Redo patterns with widgets that requires continuous editing.
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsItemDeactivatedAfterEdit ();                                       // was the last item just made inactive and made a value change when it was active? (e.g. Slider/Drag moved). Useful for Undo/Redo patterns with widgets that requires continuous editing. Note that you may get false positives (some widgets such as Combo()/ListBox()/Selectable() will return true even when clicking an already selected item).
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsItemToggledOpen          ();                                                // was the last item open state toggled? set by TreeNode().
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsAnyItemHovered           ();                                                 // is any item hovered?
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsAnyItemActive            ();                                                  // is any item active?
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsAnyItemFocused           ();                                                 // is any item focused?
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2 GetItemRectMin             ();                                                   // get upper-left bounding rectangle of the last item (screen space)
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2 GetItemRectMax             ();                                                   // get lower-right bounding rectangle of the last item (screen space)
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2 GetItemRectSize            ();                                                  // get size of last item
	 [MethodImpl(MethodImplOptions.InternalCall)] public static extern void   SetItemAllowOverlap        ();                                              // allow last item to be overlapped by a subsequent item. sometimes useful with invisible buttons, selectables, etc. to catch unused area.
	//      
    //      // Viewports
    //      // - Currently represents the Platform Window created by the application which is hosting our Dear ImGui windows.
    //      // - In 'docking' branch with multi-viewport enabled, we extend this concept to have multiple active viewports.
    //      // - In the future we will extend this concept further to also represent Platform Monitor and support a "no main platform window" operation mode.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImGuiViewport* GetMainViewport();                                                 // return primary/default viewport. This can never be NULL.
	//      
    // Miscellaneous Utilities
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool IsRectVisible(ImVec2 size);                                  // test if rectangle (of given size, starting from cursor position) is visible / not clipped.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool IsRectVisible(ImVec2 rect_min, ImVec2 rect_max);      // test if rectangle (in screen space) is visible / not clipped. to perform coarse clipping on user's side.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern double GetTime();                                                          // get global imgui time. incremented by io.DeltaTime every frame.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern int GetFrameCount();                                                    // get global imgui frame count. incremented by 1 every frame.
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImDrawList*   GetBackgroundDrawList();                                            // get background draw list for the viewport associated to the current window. this draw list will be the first rendering one. Useful to quickly draw shapes/text behind dear imgui contents.
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImDrawList*   GetForegroundDrawList();                                            // get foreground draw list for the viewport associated to the current window. this draw list will be the last rendered one. Useful to quickly draw shapes/text over dear imgui contents.
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImDrawList*   GetBackgroundDrawList(ImGuiViewport* viewport);                     // get background draw list for the given viewport. this draw list will be the first rendering one. Useful to quickly draw shapes/text behind dear imgui contents.
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImDrawList*   GetForegroundDrawList(ImGuiViewport* viewport);                     // get foreground draw list for the given viewport. this draw list will be the last rendered one. Useful to quickly draw shapes/text over dear imgui contents.
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImDrawListSharedData* GetDrawListSharedData();                                    // you may use this when creating your own ImDrawList instances.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern string GetStyleColorName(ImGuiCol idx);                                    // get a string corresponding to the enum value (for display, saving, etc.).
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetStateStorage(ImGuiStorage* storage);                             // replace current window storage with our own (if you want to manipulate it yourself, typically clear subsection of it)
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImGuiStorage* GetStateStorage();
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern void CalcListClipping(int items_count, float items_height, int* out_items_display_start, int* out_items_display_end);    // calculate coarse clipping for large list of evenly sized items. Prefer using the ImGuiListClipper higher-level helper if you can.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern bool BeginChildFrame(ImGuiID id, ImVec2 size, ImGuiWindowFlags flags = 0); // helper to create a child window / scrolling region that looks like a normal widget frame
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void EndChildFrame();                                                    // always call EndChildFrame() regardless of BeginChildFrame() return values (which indicates a collapsed/clipped window)
	//      
    //      // Text Utilities
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern  ImVec2 CalcTextSize(const char* text, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f);
	//      
    //      // Color Utilities
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec4        ColorConvertU32ToFloat4(ImU32 in);
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImU32         ColorConvertFloat4ToU32(const ImVec4& in);
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s, float& out_v);
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b);
	//      
    //      // Inputs Utilities: Keyboard
    //      // - For 'int user_key_index' you can use your own indices/enums according to how your backend/engine stored them in io.KeysDown[].
    //      // - We don't know the meaning of those value. You can use GetKeyIndex() to map a ImGuiKey_ value into the user index.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern int GetKeyIndex(ImGuiKey imgui_key);                                    // map ImGuiKey_* values into user's key index. == io.KeyMap[key]
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool IsKeyDown(int user_key_index);                                      // is key being held. == io.KeysDown[user_key_index].
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool IsKeyPressed(int user_key_index, bool repeat = true);               // was key pressed (went from !Down to Down)? if repeat=true, uses io.KeyRepeatDelay / KeyRepeatRate
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool IsKeyReleased(int user_key_index);                                  // was key released (went from Down to !Down)?
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern int GetKeyPressedAmount(int key_index, float repeat_delay, float rate); // uses provided repeat rate/delay. return a count, most often 0 or 1 but might be >1 if RepeatRate is small enough that DeltaTime > RepeatRate
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void CaptureKeyboardFromApp(bool want_capture_keyboard_value = true);    // attention: misleading name! manually override io.WantCaptureKeyboard flag next frame (said flag is entirely left for your application to handle). e.g. force capture keyboard when your widget is being hovered. This is equivalent to setting "io.WantCaptureKeyboard = want_capture_keyboard_value"; after the next NewFrame() call.
	//      
    //      // Inputs Utilities: Mouse
    //      // - To refer to a mouse button, you may use named enums in your code e.g. ImGuiMouseButton_Left, ImGuiMouseButton_Right.
    //      // - You can also use regular integer: it is forever guaranteed that 0=Left, 1=Right, 2=Middle.
    //      // - Dragging operations are only reported after mouse has moved a certain distance away from the initial clicking position (see 'lock_threshold' and 'io.MouseDraggingThreshold')
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsMouseDown         (ImGuiMouseButton button);                               // is mouse button held?
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsMouseClicked      (ImGuiMouseButton button, bool repeat = false);       // did mouse button clicked? (went from !Down to Down)
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsMouseReleased     (ImGuiMouseButton button);                           // did mouse button released? (went from Down to !Down)
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsMouseDoubleClicked(ImGuiMouseButton button);                      // did mouse button double-clicked? (note that a double-click will also report IsMouseClicked() == true)
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsMouseHoveringRect (ImVec2 r_min, ImVec2 r_max, bool clip = true);// is mouse hovering given bounding rect (in screen space). clipped by current clipping settings, but disregarding of other consideration of focus/window ordering/popup-block.
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsMousePosValid     (ImVec2 mouse_pos);                    // by convention we use (-FLT_MAX,-FLT_MAX) to denote that there is no mouse available
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsAnyMouseDown();                                                   // is any mouse button held?
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2 GetMousePos();                                                      // shortcut to ImGui::GetIO().MousePos provided by user, to be consistent with other calls
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2 GetMousePosOnOpeningCurrentPopup();                                 // retrieve mouse position at the time of opening popup we have BeginPopup() into (helper to avoid user backing that value themselves)
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool   IsMouseDragging(ImGuiMouseButton button, float lock_threshold = -1.0f);         // is mouse dragging? (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImVec2 GetMouseDragDelta(ImGuiMouseButton button = 0, float lock_threshold = -1.0f);   // return the delta from the initial clicking position while the mouse button is pressed or was just released. This is locked and return 0.0f until the mouse moves past a distance threshold at least once (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void   ResetMouseDragDelta(ImGuiMouseButton button = 0);                   //
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImGuiMouseCursor GetMouseCursor();                                                // get desired cursor type, reset in ImGui::NewFrame(), this is updated during the frame. valid before Render(). If you use software rendering by setting io.MouseDrawCursor ImGui will render those for you
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetMouseCursor(ImGuiMouseCursor cursor_type);                       // set desired cursor type
	//      [MethodImpl(MethodImplOptions.InternalCall)] public static extern void CaptureMouseFromApp(bool want_capture_mouse_value = true);          // attention: misleading name! manually override io.WantCaptureMouse flag next frame (said flag is entirely left for your application to handle). This is equivalent to setting "io.WantCaptureMouse = want_capture_mouse_value;" after the next NewFrame() call.
	//      
    //      // Clipboard Utilities
    // - Also see the LogToClipboard() function to capture GUI into clipboard, or easily output text data to the clipboard.
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern string GetClipboardText();
	[MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetClipboardText(string text);

    // Settings/.Ini Utilities
    // - The disk functions are automatically called if io.IniFilename != NULL (default is "imgui.ini").
    // - Set io.IniFilename to NULL to load/save manually. Read io.WantSaveIniSettings description about handling .ini saving manually.
    // - Important: default value "imgui.ini" is relative to current working dir! Most apps will want to lock this to an absolute path (e.g. same path as executables).
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern void LoadIniSettingsFromDisk(const char* ini_filename);                  // call after CreateContext() and before the first call to NewFrame(). NewFrame() automatically calls LoadIniSettingsFromDisk(io.IniFilename).
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern void LoadIniSettingsFromMemory(const char* ini_data, size_t ini_size=0); // call after CreateContext() and before the first call to NewFrame() to provide .ini data from your own data source.
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SaveIniSettingsToDisk(const char* ini_filename);                    // this is automatically called (if io.IniFilename is not empty) a few seconds after any modification that should be reflected in the .ini file (and also by DestroyContext).
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern const char* SaveIniSettingsToMemory(size_t * out_ini_size = NULL);               // return a zero-terminated string with the .ini data which you can save by your own mean. call when io.WantSaveIniSettings is set, then save data by your own mean and clear io.WantSaveIniSettings.

    // Debug Utilities
    // - This is used by the IMGUI_CHECKVERSION() macro.
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern bool DebugCheckVersionAndDataLayout(const char* version_str, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_drawvert, size_t sz_drawidx); // This is called by IMGUI_CHECKVERSION() macro.

    // Memory Allocators
    // - Those functions are not reliant on the current context.
    // - DLL users: heaps and globals are not shared across DLL boundaries! You will need to call SetCurrentContext() + SetAllocatorFunctions()
    //   for each static/DLL boundary you are calling from. Read "Context and Memory Allocators" section of imgui.cpp for more details.
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern void SetAllocatorFunctions(ImGuiMemAllocFunc alloc_func, ImGuiMemFreeFunc free_func, void* user_data = NULL);
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern void GetAllocatorFunctions(ImGuiMemAllocFunc* p_alloc_func, ImGuiMemFreeFunc* p_free_func, void** p_user_data);
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern void* MemAlloc(size_t size);
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern void  MemFree(void* ptr);

    // (Optional) Platform/OS interface for multi-viewport support
    // Read comments around the ImGuiPlatformIO structure for more details.
    // Note: You may use GetWindowViewport() to get the current viewport of the current window.
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImGuiViewport*    FindViewportByID(ImGuiID id);                                   // this is a helper for backends.
	// [MethodImpl(MethodImplOptions.InternalCall)] public static extern ImGuiViewport*    FindViewportByPlatformHandle(void* platform_handle);            // this is a helper for backends. the type platform_handle is decided by the backend (e.g. HWND, MyWindow*, GLFWwindow* etc.)
}

 */
