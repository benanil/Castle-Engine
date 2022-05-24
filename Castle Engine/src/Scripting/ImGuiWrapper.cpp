#include "ImGuiWrapper.hpp"
#include "ScriptingEngine.hpp"
#include <imgui.h>
//#include "../Structures/HString.hpp"
/*
namespace ImGui
{
	struct ImIvec2 {
		int x, y;
		ImIvec2(int _x, int _y) : x(_x), y(_y) { }
	};
	
	struct ImIvec3 {
		int x, y, z;
		ImIvec3(int _x, int _y, int _z) : x(_x), y(_y), z(_z) { }
	};
	
	struct ImIvec4 {
		int x, y, z, w;
		ImIvec4(int _x, int _y, int _z, int _w) : x(_x), y(_y), z(_z), w(_w) { }
	};
}
namespace CImGui
{
	struct ImString {
		char* cstr;
		ImString(MonoString* name) : cstr(HS::ToCharArray(mono_string_chars(name))) { }
		~ImString() { free(cstr); }
	};

	__declspec(dllexport) bool Begin(MonoString* name, bool p_open, ImGuiWindowFlags flags) {
		return ImGui::Begin(ImString(name).cstr, &p_open, flags) ;
	}

	__declspec(dllexport) bool BeginChild(MonoString* str_id, ImVec2 size, bool border, ImGuiWindowFlags flags) {
		return ImGui::BeginChild(ImString(str_id).cstr, size, border, flags);
	}
	
	__declspec(dllexport) bool BeginChild(ImGuiID id, ImVec2 size, bool border, ImGuiWindowFlags flags) {
		return ImGui::BeginChild(id, size, border, flags);
	}
	
	__declspec(dllexport) void EndChild() { ImGui::EndChild(); }

	__declspec(dllexport) void IsWindowAppearing() { ImGui::IsWindowAppearing(); }
	__declspec(dllexport) void IsWindowCollapsed() { ImGui::IsWindowCollapsed(); }
	__declspec(dllexport) bool IsWindowFocused(ImGuiFocusedFlags flags) { return ImGui::IsWindowFocused(); }
	__declspec(dllexport) bool IsWindowHovered(ImGuiHoveredFlags flags) { return ImGui::IsWindowHovered(); }
	
	__declspec(dllexport) float         GetWindowDpiScale() { return ImGui::GetWindowDpiScale(); }
	__declspec(dllexport) ImVec2        GetWindowPos     () { return ImGui::GetWindowPos     (); }
	__declspec(dllexport) ImVec2        GetWindowSize    () { return ImGui::GetWindowSize    (); }
	__declspec(dllexport) float         GetWindowWidth   () { return ImGui::GetWindowWidth   (); }
	__declspec(dllexport) float         GetWindowHeight  () { return ImGui::GetWindowHeight  (); }
	//__declspec(dllexport) ImGuiViewport GetWindowViewport() { return ImGui::GetWindowViewport(); }

	__declspec(dllexport) void SetNextWindowPos        (ImVec2 pos, ImGuiCond cond, ImVec2 pivot) {}
	__declspec(dllexport) void SetNextWindowSize       (ImVec2 size, ImGuiCond cond )			  {}

	// __declspec(dllexport) void SetNextWindowContentSize(ImVec2 size) 					{ ImGui::SetNextWindowContentSize(); }
	// __declspec(dllexport) void SetNextWindowCollapsed  (bool collapsed, ImGuiCond cond) { ImGui::SetNextWindowCollapsed  (); }
	// __declspec(dllexport) void SetNextWindowViewport   (ImGuiID viewport_id)            { ImGui::SetNextWindowViewport   (); }
	// __declspec(dllexport) void SetWindowPos            (ImVec2 pos, ImGuiCond cond)     { ImGui::SetWindowPos            (); }
	// __declspec(dllexport) void SetWindowSize           (ImVec2 size, ImGuiCond cond)    { ImGui::SetWindowSize           (); }
	// __declspec(dllexport) void SetWindowCollapsed      (bool collapsed, ImGuiCond cond) { ImGui::SetWindowCollapsed      (); }
	
	__declspec(dllexport) void SetWindowPos (MonoString* name, ImVec2 pos, ImGuiCond cond) {
		ImGui::SetWindowPos(ImString(name).cstr, pos, cond); 
	}
	
	__declspec(dllexport) void SetWindowSize (MonoString* name, ImVec2 size, ImGuiCond cond) { 
		ImGui::SetWindowSize(ImString(name).cstr, size, cond); 
	}
	
	__declspec(dllexport) void SetWindowCollapsed (MonoString* name, bool collapsed, ImGuiCond cond) {
		ImGui::SetWindowCollapsed(ImString(name).cstr, collapsed, cond); 
	}
	
	__declspec(dllexport) void SetWindowFocus (MonoString* name) { ImGui::SetWindowFocus(ImString(name).cstr);  }

	__declspec(dllexport) void SetNextWindowFocus  ()           { ImGui::SetNextWindowFocus  ();      }
	__declspec(dllexport) void SetNextWindowBgAlpha(float alpha){ ImGui::SetNextWindowBgAlpha(alpha); }
	__declspec(dllexport) void SetWindowFontScale  (float scale){ ImGui::SetWindowFontScale  (scale); }
	__declspec(dllexport) void SetWindowFocus      ()			{ ImGui::SetWindowFocus      (); 	  }                                               

	__declspec(dllexport) ImVec2  GetContentRegionAvail    () { return ImGui::GetContentRegionAvail    (); }
	__declspec(dllexport) ImVec2  GetContentRegionMax      () { return ImGui::GetContentRegionMax      (); }
	__declspec(dllexport) ImVec2  GetWindowContentRegionMin() { return ImGui::GetWindowContentRegionMin(); }
	__declspec(dllexport) ImVec2  GetWindowContentRegionMax() { return ImGui::GetWindowContentRegionMax(); }

	__declspec(dllexport) float GetScrollX       ()								              { return ImGui::GetScrollX(); }
	__declspec(dllexport) float GetScrollY       ()								              { return ImGui::GetScrollX(); }               
	__declspec(dllexport) void  SetScrollX       (float scroll_x)                             { ImGui::SetScrollX(scroll_x); }
	__declspec(dllexport) void  SetScrollY       (float scroll_y)                             { ImGui::SetScrollY(scroll_y); }
	__declspec(dllexport) float GetScrollMaxX    ()                                           { return  ImGui::GetScrollMaxX(); }              
	__declspec(dllexport) float GetScrollMaxY    ()                                           { return  ImGui::GetScrollMaxY(); }              
	__declspec(dllexport) void  SetScrollHereX   (float center_x_ratio)                { ImGui::SetScrollHereX   (center_x_ratio); }
	__declspec(dllexport) void  SetScrollHereY   (float center_y_ratio)                { ImGui::SetScrollHereY   (center_y_ratio); }
	__declspec(dllexport) void  SetScrollFromPosX(float local_x, float center_x_ratio) { ImGui::SetScrollFromPosX(local_x, center_x_ratio); }
	__declspec(dllexport) void  SetScrollFromPosY(float local_y, float center_y_ratio) { ImGui::SetScrollFromPosY(local_y, center_y_ratio); }

	// __declspec(dllexport) void PushFont(ImFont* font) {}
	__declspec(dllexport) void PopFont	              ()                              { ImGui::PopFont	             (); 					 }
	__declspec(dllexport) void PushStyleColor         (ImGuiCol idx, ImU32 col)       { ImGui::PushStyleColor        (idx, col); 			 }
	__declspec(dllexport) void PushStyleColor         (ImGuiCol idx, ImVec4 col)      { ImGui::PushStyleColor        (idx, col); 			 }
	__declspec(dllexport) void PopStyleColor          (int count)                     { ImGui::PopStyleColor         (count); 				 }
	__declspec(dllexport) void PushStyleVar           (ImGuiStyleVar idx, float val)  { ImGui::PushStyleVar          (idx, val); 			 }
	__declspec(dllexport) void PushStyleVar           (ImGuiStyleVar idx, ImVec2 val) { ImGui::PushStyleVar          (idx, val); 			 }
	__declspec(dllexport) void PopStyleVar            (int count)                 	  { ImGui::PopStyleVar           (count); 				 }
	__declspec(dllexport) void PushAllowKeyboardFocus (bool allow_keyboard_focus)     { ImGui::PushAllowKeyboardFocus(allow_keyboard_focus); }
	__declspec(dllexport) void PopAllowKeyboardFocus  ()                              { ImGui::PopAllowKeyboardFocus (); 				     }
	__declspec(dllexport) void PushButtonRepeat       (bool repeat)                   { ImGui::PushButtonRepeat      (repeat); 			     }
	__declspec(dllexport) void PopButtonRepeat        ()							  { ImGui::PopButtonRepeat       (); 				     }                             

	__declspec(dllexport) void  PushItemWidth    (float item_width)        { ImGui::PushItemWidth   (item_width); }
	__declspec(dllexport) void  PopItemWidth     ()                        { ImGui::PopItemWidth    (); }
	__declspec(dllexport) void  SetNextItemWidth (float item_width)        { ImGui::SetNextItemWidth(item_width); }
	__declspec(dllexport) float CalcItemWidth    ()                        { return ImGui::CalcItemWidth(); }
	__declspec(dllexport) void  PushTextWrapPos  (float wrap_local_pos_x)  { ImGui::PushTextWrapPos (wrap_local_pos_x); }
	__declspec(dllexport) void  PopTextWrapPos   ()                        { ImGui::PopTextWrapPos  (); }

	__declspec(dllexport) float   GetFontSize           ()                              { return ImGui::GetFontSize           (); 				}       
	__declspec(dllexport) ImVec2  GetFontTexUvWhitePixel()                              { return ImGui::GetFontTexUvWhitePixel(); 				}       
	__declspec(dllexport) ImU32   GetColorU32           (ImGuiCol idx, float alpha_mul) { return ImGui::GetColorU32           (idx, alpha_mul); }
	__declspec(dllexport) ImU32   GetColorU32           (ImVec4 col)                    { return ImGui::GetColorU32           (col); 			}       
	__declspec(dllexport) ImU32   GetColorU32           (ImU32 col)                     { return ImGui::GetColorU32           (col); 			}                               
	__declspec(dllexport) ImVec4  GetStyleColorVec4     (ImGuiCol idx)                  { return ImGui::GetStyleColorVec4     (idx); 			}

	__declspec(dllexport) void 	  Separator					  () 										 { ImGui::Separator();   				}
	__declspec(dllexport) void    SameLine 					  (float offset_from_start_x, float spacing) { ImGui::SameLine (offset_from_start_x, spacing);   } 
	__declspec(dllexport) void    NewLine                     ()                                        { ImGui::NewLine();   					} 
	__declspec(dllexport) void    Spacing                     ()                                        { ImGui::Spacing();   					} 
	__declspec(dllexport) void    Dummy					      (ImVec2 size)                             { ImGui::Dummy	(size);   				} 
	__declspec(dllexport) void    Indent					  (float indent_w)                          { ImGui::Indent	(indent_w);   			} 
	__declspec(dllexport) void    Unindent				      (float indent_w)                          { ImGui::Unindent(indent_w);   			} 
	__declspec(dllexport) void    BeginGroup                  ()                                        { ImGui::BeginGroup();   				} 
	__declspec(dllexport) void    EndGroup					  ()                                        { ImGui::EndGroup();   					} 
	__declspec(dllexport) ImVec2  GetCursorPos	              ()                                        { return ImGui::GetCursorPos();   		} 
	__declspec(dllexport) float   GetCursorPosX				  ()                                        { return ImGui::GetCursorPosX();   		} 
	__declspec(dllexport) float   GetCursorPosY				  ()                                        { return ImGui::GetCursorPosY();   		} 
	__declspec(dllexport) void    SetCursorPos	  			  (ImVec2 local_pos)                        { ImGui::SetCursorPos(local_pos);   	} 
	__declspec(dllexport) void    SetCursorPosX				  (float local_x)                           { ImGui::SetCursorPosX(local_x);   		} 
	__declspec(dllexport) void    SetCursorPosY				  (float local_y)                           { ImGui::SetCursorPosY(local_y);   		} 
	__declspec(dllexport) ImVec2  GetCursorStartPos	          ()                                        { return ImGui::GetCursorStartPos();   	} 
	__declspec(dllexport) ImVec2  GetCursorScreenPos		  ()                                        { return ImGui::GetCursorScreenPos();   } 
	__declspec(dllexport) void    SetCursorScreenPos		  (ImVec2 pos)                              { ImGui::SetCursorScreenPos(pos);   	} 
	__declspec(dllexport) void    AlignTextToFramePadding     ()                                        { ImGui::AlignTextToFramePadding();   	} 
	__declspec(dllexport) float   GetTextLineHeight           ()                                        { return ImGui::GetTextLineHeight           ();   } 
	__declspec(dllexport) float   GetTextLineHeightWithSpacing()                                        { return ImGui::GetTextLineHeightWithSpacing();   } 
	__declspec(dllexport) float   GetFrameHeight			  ()                                        { return ImGui::GetFrameHeight			  	();   } 
	__declspec(dllexport) float GetFrameHeightWithSpacing   ()                                          { return ImGui::GetFrameHeightWithSpacing   ();   }

	__declspec(dllexport) void PushID(MonoString* str_id)   { ImGui::PushID(ImString(str_id).cstr); }
	__declspec(dllexport) void PushID(int int_id)           { ImGui::PushID(int_id); }
	__declspec(dllexport) void PopID()                      { ImGui::PopID();  }
	// __declspec(dllexport) ImGuiID GetID(MonoString* str_id) { return ImGui::GetID(ImString(st_id).cstr); }

	__declspec(dllexport) void Text           (MonoString* fmt)                     { ImGui::Text(ImString(fmt).cstr); 								}
	__declspec(dllexport) void TextColored    (ImVec4 col, MonoString* fmt)         { ImGui::TextColored(col, ImString(fmt).cstr); 					}
	__declspec(dllexport) void TextDisabled   (MonoString* fmt)                     { ImGui::TextDisabled(ImString(fmt).cstr); 						}
	__declspec(dllexport) void TextWrapped    (MonoString* fmt)                     { ImGui::TextWrapped(ImString(fmt).cstr); 						}
	__declspec(dllexport) void LabelText      (MonoString* label, MonoString* fmt)  { ImGui::LabelText(ImString(label).cstr, ImString(fmt).cstr); 	}
	__declspec(dllexport) void BulletText     (MonoString* fmt)                     { ImGui::BulletText(ImString(fmt).cstr); 					  	}

	// __declspec(dllexport) bool Button         (MonoString* label, ImVec2 size)									 { return ImGui::Button			(ImString(label).cstr , size); }   // button
	// __declspec(dllexport) bool SmallButton    (MonoString* label)												 { return ImGui::SmallButton    (ImString(label).cstr ); } // button with FramePadding=(0,0) to easily embed within text
	// __declspec(dllexport) bool InvisibleButton(MonoString* str_id, ImVec2 size, ImGuiButtonFlags flags = 0)		 { return ImGui::InvisibleButton(ImString(str_id).cstr, size, flags); } // flexible button behavior without the visuals, frequently useful to bu
	// __declspec(dllexport) bool ArrowButton    (MonoString* str_id, ImGuiDir dir)								 { return ImGui::ArrowButton    (ImString(str_id).cstr, dir); }                  // square button with an arrow shape
	// __declspec(dllexport) bool Checkbox       (MonoString* label, bool v)										 { return ImGui::Checkbox       (ImString(label).cstr , &v); }
	// __declspec(dllexport) bool CheckboxFlags  (MonoString* label, int flags, int flags_value)					 { return ImGui::CheckboxFlags  (ImString(label).cstr , flags); }
	// __declspec(dllexport) bool CheckboxFlags  (MonoString* label, unsigned int flags, unsigned int flags_value)  { return ImGui::CheckboxFlags  (ImString(label).cstr , &flags, flags_value); }
	// __declspec(dllexport) bool RadioButton    (MonoString* label, bool active)									 { return ImGui::RadioButton    (ImString(label).cstr , active); } 	// use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; }
	// __declspec(dllexport) bool RadioButton    (MonoString* label, int v, int v_button)							 { return ImGui::RadioButton    (ImString(label).cstr, &v, v_button); }           // shortcut to handle the above pattern when value is an integer
	// __declspec(dllexport) void ProgressBar    (float fraction, ImVec2 size_arg, MonoString* overlay) 		     { return ImGui::ProgressBar    (fraction, size_arg, ImString(overlay).cstr); } // size_arg = ImVec2(-FLT_MIN, 0)
	
	// __declspec(dllexport) void Image          (ImTextureID user_texture_id, ImVec2 size, ImVec2 uv0, ImVec2 uv1, ImVec4 tint_col, ImVec4 border_col) {
	// 	ImGui::Image(user_texture_id, size, uv0, uv1, tint_col, bg_col);
	// }
	// __declspec(dllexport) bool ImageButton(ImTextureID user_texture_id, ImVec2 size, ImVec2 uv0, ImVec2 uv1, int frame_padding, ImVec4 bg_col, ImVec4 tint_col) {
	// 	ImGui::ImageButton(user_texture_id, size, uv0, uv1, frame_padding, bg_col, tint_col);
	// }
	// 
	// __declspec(dllexport) void Bullet() { ImGui::Bullet(); } // draw a small circle + keep the cursor on the same line. advance cursor x posit
	// 
	// __declspec(dllexport) bool BeginCombo(MonoString* label, MonoString* preview_value, ImGuiComboFlags flags)  {
	// 	return ImGui::BeginCombo(ImString(label).cstr, ImString(preview_value).cstr, flags); 
	// }
	// __declspec(dllexport) void EndCombo(); // only call EndCombo() if BeginCombo() returns true!
	// __declspec(dllexport) bool Combo(MonoString* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
	// __declspec(dllexport) bool Combo(MonoString* label, int* current_item, MonoString* items_separated_by_zeros, int popup_max_height_in_items = -1);      // Separate items with \0 within a string, end item-list wi
	// __declspec(dllexport) bool Combo(MonoString* label, int* current_item, bool (* items_getter) (void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items = -1);

	// __declspec(dllexport) bool DragFloat  (MonoString* label, float v   , float v_speed, float v_min, float v_max, MonoString* format, ImGuiSliderFlags flags);     // If v_min >= v_max we have no bound
	// __declspec(dllexport) bool DragFloat2 (MonoString* label, ImVec2 v  , float v_speed, float v_min, float v_max, MonoString* format, ImGuiSliderFlags flags);
	// __declspec(dllexport) bool DragFloat3 (MonoString* label, ImVec3 v  , float v_speed, float v_min, float v_max, MonoString* format, ImGuiSliderFlags flags);
	// __declspec(dllexport) bool DragFloat4 (MonoString* label, ImVec4 v  , float v_speed, float v_min, float v_max, MonoString* format, ImGuiSliderFlags flags);
	// __declspec(dllexport) bool DragInt    (MonoString* label, int v     , float v_speed, int v_min, int v_max, MonoString* format, ImGuiSliderFlags flags);  // If v_min >= v_max we have no bound
	// __declspec(dllexport) bool DragInt2   (MonoString* label, ImIVec2 v , float v_speed, int v_min, int v_max, MonoString* format, ImGuiSliderFlags flags);
	// __declspec(dllexport) bool DragInt3   (MonoString* label, ImIVec3 v , float v_speed, int v_min, int v_max, MonoString* format, ImGuiSliderFlags flags);
	// __declspec(dllexport) bool DragInt4   (MonoString* label, ImIVec4 v , float v_speed, int v_min, int v_max, MonoString* format, ImGuiSliderFlags flags);
	// 
	// __declspec(dllexport) bool SliderFloat  (MonoString* label, float v   , float v_speed, float v_min, float v_max, MonoString* format, ImGuiSliderFlags flags);     // If v_min >= v_max we have no bound
	// __declspec(dllexport) bool SliderFloat2 (MonoString* label, ImVec2 v  , float v_speed, float v_min, float v_max, MonoString* format, ImGuiSliderFlags flags);
	// __declspec(dllexport) bool SliderFloat3 (MonoString* label, ImVec3 v  , float v_speed, float v_min, float v_max, MonoString* format, ImGuiSliderFlags flags);
	// __declspec(dllexport) bool SliderFloat4 (MonoString* label, ImVec4 v  , float v_speed, float v_min, float v_max, MonoString* format, ImGuiSliderFlags flags);
	// __declspec(dllexport) bool SliderInt    (MonoString* label, int v     , float v_speed, int v_min, int v_max, MonoString* format, ImGuiSliderFlags flags);  // If v_min >= v_max we have no bound
	// __declspec(dllexport) bool SliderInt2   (MonoString* label, ImIVec2 v , float v_speed, int v_min, int v_max, MonoString* format, ImGuiSliderFlags flags);
	// __declspec(dllexport) bool SliderInt3   (MonoString* label, ImIVec3 v , float v_speed, int v_min, int v_max, MonoString* format, ImGuiSliderFlags flags);
	// __declspec(dllexport) bool SliderInt4   (MonoString* label, ImIVec4 v , float v_speed, int v_min, int v_max, MonoString* format, ImGuiSliderFlags flags);
	// 
	// __declspec(dllexport) bool InputText         (MonoString* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	// __declspec(dllexport) bool InputTextMultiline(MonoString* label, char* buf, size_t buf_size, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	// __declspec(dllexport) bool InputTextWithHint (MonoString* label, MonoString* hint, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	// __declspec(dllexport) bool InputFloat        (MonoString* label, float v, float step, float step_fast, MonoString* format, ImGuiInputTextFlags flags );
	// __declspec(dllexport) bool InputFloat2       (MonoString* label, ImVec2 v, MonoString* format, ImGuiInputTextFlags flags);
	// __declspec(dllexport) bool InputFloat3       (MonoString* label, ImVec3 v, MonoString* format, ImGuiInputTextFlags flags);
	// __declspec(dllexport) bool InputFloat4       (MonoString* label, ImVec4 v, MonoString* format, ImGuiInputTextFlags flags);
	// __declspec(dllexport) bool InputInt          (MonoString* label, int* v, int step, int step_fast, ImGuiInputTextFlags flags);
	// __declspec(dllexport) bool InputInt2         (MonoString* label, ImIVec2 v, ImGuiInputTextFlags flags);
	// __declspec(dllexport) bool InputInt3         (MonoString* label, ImIVec3 v, ImGuiInputTextFlags flags);
	// __declspec(dllexport) bool InputInt4         (MonoString* label, ImIVec4 v, ImGuiInputTextFlags flags);
	// __declspec(dllexport) bool InputDouble       (MonoString* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0);
	// 
	// __declspec(dllexport) bool ColorEdit3  (MonoString* label, float col[3], ImGuiColorEditFlags flags = 0);
	// __declspec(dllexport) bool ColorEdit4  (MonoString* label, float col[4], ImGuiColorEditFlags flags = 0);
	// __declspec(dllexport) bool ColorPicker3(MonoString* label, float col[3], ImGuiColorEditFlags flags = 0);
	// __declspec(dllexport) bool ColorPicker4(MonoString* label, float col[4], ImGuiColorEditFlags flags = 0, const float* ref_col = NULL);
	// __declspec(dllexport) bool ColorButton (MonoString* desc_id, const ImVec4& col, ImGuiColorEditFlags flags = 0, ImVec2 size = ImVec2(0, 0)); // display
	// __declspec(dllexport) void SetColorEditOptions(ImGuiColorEditFlags flags);                     // initialize current options (generally on applic
	// 
	// 
	// __declspec(dllexport) bool TreeNode   (MonoString* label);
	// __declspec(dllexport) bool TreeNode   (MonoString* str_id, string fmt);   // helper variation to easily decorelate the id from the displayed stri
	// __declspec(dllexport) bool TreeNodeEx (MonoString* label, ImGuiTreeNodeFlags flags = 0);
	// __declspec(dllexport) void TreePush   (string str_id);                                       // ~ Indent()+PushId(). Already called by TreeNode()
	// __declspec(dllexport) void TreePop();                                                          // ~ Unindent()+PopId()
	// __declspec(dllexport) float GetTreeNodeToLabelSpacing();                                        // horizontal distance preceding label when using
	// __declspec(dllexport) bool CollapsingHeader(MonoString* label, ImGuiTreeNodeFlags flags = 0);  // if returning 'true' the header is open. doesn't inde
	// __declspec(dllexport) bool CollapsingHeader(MonoString* label, bool* p_visible, ImGuiTreeNodeFlags flags = 0); // when 'p_visible != NULL': if '*p_vis
	// __declspec(dllexport) void SetNextItemOpen (bool is_open, ImGuiCond cond = 0);                  // set next TreeNode/CollapsingHeader open state.

	// __declspec(dllexport) bool DragFloatRange2(string label, float* v_current_min, float* v_current_max, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", const char* format_max = NULL, ImGuiSliderFlags flags = 0);
	// __declspec(dllexport) bool DragIntRange2  (string label, int* v_current_min, int* v_current_max, float v_speed = 1.0f, int v_min = 0, int v_max = 0, string format = "%d", string format_max = NULL, ImGuiSliderFlags flags = 0);

}

namespace ImGuiWrapper
{


	void InitializeInternalCalls()
	{
		// mono_add_internal_call("CImGui::Begin", &ImGui::Begin);
		// mono_add_internal_call("CImGui::End", &ImGui::End);
		// 
		// // mono_add_internal_call("ImGui::BeginChild", &ImGui::BeginChild);
		// mono_add_internal_call("CImGui::EndChild", &ImGui::EndChild);
		// 
		// mono_add_internal_call("CImGui::GetWindowDpiScale" , &CImGui::GetWindowDpiScale );
		// mono_add_internal_call("CImGui::GetWindowPos"      , &CImGui::GetWindowPos      );
		// mono_add_internal_call("CImGui::GetWindowSize"     , &CImGui::GetWindowSize     );
		// mono_add_internal_call("CImGui::GetWindowWidth"    , &CImGui::GetWindowWidth    );
		// mono_add_internal_call("CImGui::GetWindowHeight"   , &CImGui::GetWindowHeight   );
		// //mono_add_internal_call("CImGui::GetWindowViewport" , &CImGui::GetWindowViewport );
		// 
		// mono_add_internal_call("CImGui::GetContentRegionAvail"    , &CImGui::GetContentRegionAvail    );
		// mono_add_internal_call("CImGui::GetContentRegionMax"      , &CImGui::GetContentRegionMax      );
		// mono_add_internal_call("CImGui::GetWindowContentRegionMin", &CImGui::GetWindowContentRegionMin);
		// mono_add_internal_call("CImGui::GetWindowContentRegionMax", &CImGui::GetWindowContentRegionMax);
		// 
		// mono_add_internal_call("CImGui::GetScrollX"       , &CImGui::GetScrollX       );
		// mono_add_internal_call("CImGui::GetScrollY"       , &CImGui::GetScrollY       );
		// mono_add_internal_call("CImGui::SetScrollX"       , &CImGui::SetScrollX       );
		// mono_add_internal_call("CImGui::SetScrollY"       , &CImGui::SetScrollY       );
		// mono_add_internal_call("CImGui::GetScrollMaxX"    , &CImGui::GetScrollMaxX    );
		// mono_add_internal_call("CImGui::GetScrollMaxY"    , &CImGui::GetScrollMaxY    );
		// mono_add_internal_call("CImGui::SetScrollHereX"   , &CImGui::SetScrollHereX   );
		// mono_add_internal_call("CImGui::SetScrollHereY"   , &CImGui::SetScrollHereY   );
		// mono_add_internal_call("CImGui::SetScrollFromPosX", &CImGui::SetScrollFromPosX);
		// mono_add_internal_call("CImGui::SetScrollFromPosY", &CImGui::SetScrollFromPosY);
		// 
		// mono_add_internal_call("CImGui::PopFont"                , &CImGui::PopFont	           );
		// mono_add_internal_call("CImGui::PushStyleColor"         , &CImGui::PushStyleColor        );
		// mono_add_internal_call("CImGui::PushStyleColor"         , &CImGui::PushStyleColor        );
		// mono_add_internal_call("CImGui::PopStyleColor"          , &CImGui::PopStyleColor         );
		// mono_add_internal_call("CImGui::PushStyleVar"           , &CImGui::PushStyleVar          );
		// mono_add_internal_call("CImGui::PushStyleVar"           , &CImGui::PushStyleVar          );
		// mono_add_internal_call("CImGui::PopStyleVar"            , &CImGui::PopStyleVar           );
		// mono_add_internal_call("CImGui::PushAllowKeyboardFocus" , &CImGui::PushAllowKeyboardFocus);
		// mono_add_internal_call("CImGui::PopAllowKeyboardFocus " , &CImGui::PopAllowKeyboardFocus );
		// mono_add_internal_call("CImGui::PushButtonRepeat"       , &CImGui::PushButtonRepeat      );
		// mono_add_internal_call("CImGui::PopButtonRepeat"        , &CImGui::PopButtonRepeat       );
		// 
		// mono_add_internal_call("CImGui::PushItemWidth"    , &CImGui::PushItemWidth   );
		// mono_add_internal_call("CImGui::PopItemWidth"     , &CImGui::PopItemWidth    );
		// mono_add_internal_call("CImGui::SetNextItemWidth" , &CImGui::SetNextItemWidth);
		// mono_add_internal_call("CImGui::CalcItemWidth"    , &CImGui::CalcItemWidth   );
		// mono_add_internal_call("CImGui::PushTextWrapPos"  , &CImGui::PushTextWrapPos );
		// mono_add_internal_call("CImGui::PopTextWrapPos"   , &CImGui::PopTextWrapPos  );
		// 
		// mono_add_internal_call("CImGui::GetFontSize"           , &CImGui::GetFontSize           );
		// mono_add_internal_call("CImGui::GetFontTexUvWhitePixel", &CImGui::GetFontTexUvWhitePixel);
		// mono_add_internal_call("CImGui::GetColorU32"           , &CImGui::GetColorU32           );
		// mono_add_internal_call("CImGui::GetColorU32"           , &CImGui::GetColorU32           );
		// mono_add_internal_call("CImGui::GetColorU32"           , &CImGui::GetColorU32           );
		// mono_add_internal_call("CImGui::GetStyleColorVec4"     , &CImGui::GetStyleColorVec4     );
		// 
		// mono_add_internal_call("CImGui::Separator",     	         	 &CImGui::Separator					  );
		// mono_add_internal_call("CImGui::SameLine",      	         	 &CImGui::SameLine 					  );
		// mono_add_internal_call("CImGui::NewLine",       	         	 &CImGui::NewLine                      );
		// mono_add_internal_call("CImGui::Spacing",       	         	 &CImGui::Spacing                      );
		// mono_add_internal_call("CImGui::Dummy",         	         	 &CImGui::Dummy					      );
		// mono_add_internal_call("CImGui::Indent",        	         	 &CImGui::Indent					  	  );
		// mono_add_internal_call("CImGui::Unindent",      	         	 &CImGui::Unindent				      );
		// mono_add_internal_call("CImGui::BeginGroup",    	         	 &CImGui::BeginGroup                   );
		// mono_add_internal_call("CImGui::EndGroup",      	         	 &CImGui::EndGroup					  );
		// mono_add_internal_call("CImGui::GetCursorPos",  	         	 &CImGui::GetCursorPos	              );
		// mono_add_internal_call("CImGui::GetCursorPosX", 	         	 &CImGui::GetCursorPosX				  );
		// mono_add_internal_call("CImGui::GetCursorPosY", 	         	 &CImGui::GetCursorPosY				  );
		// mono_add_internal_call("CImGui::SetCursorPos",  	         	 &CImGui::SetCursorPos	  			  );
		// mono_add_internal_call("CImGui::SetCursorPosX", 	         	 &CImGui::SetCursorPosX				  );
		// mono_add_internal_call("CImGui::SetCursorPosY", 	         	 &CImGui::SetCursorPosY				  );
		// mono_add_internal_call("CImGui::GetCursorStartPos",       	     &CImGui::GetCursorStartPos	          );
		// mono_add_internal_call("CImGui::GetCursorScreenPos",      	     &CImGui::GetCursorScreenPos		  	  );
		// mono_add_internal_call("CImGui::SetCursorScreenPos",      	     &CImGui::SetCursorScreenPos		  	  );
		// mono_add_internal_call("CImGui::AlignTextToFramePadding", 	     &CImGui::AlignTextToFramePadding      );
		// mono_add_internal_call("CImGui::GetTextLineHeight",       	     &CImGui::GetTextLineHeight            );
		// mono_add_internal_call("CImGui::GetTextLineHeightWithSpacing",   &CImGui::GetTextLineHeightWithSpacing );
		

		// mono_add_internal_call("CImGui::IsItemHovered"              , &CImGui::IsItemHovered             );
		// mono_add_internal_call("CImGui::IsItemActive"               , &CImGui::IsItemActive              );
		// mono_add_internal_call("CImGui::IsItemFocused"              , &CImGui::IsItemFocused             );
		// mono_add_internal_call("CImGui::IsItemClicked"              , &CImGui::IsItemClicked             );
		// mono_add_internal_call("CImGui::IsItemVisible"              , &CImGui::IsItemVisible             );
		// mono_add_internal_call("CImGui::IsItemEdited"               , &CImGui::IsItemEdited              );
		// mono_add_internal_call("CImGui::IsItemActivated"            , &CImGui::IsItemActivated           );
		// mono_add_internal_call("CImGui::IsItemDeactivated"          , &CImGui::IsItemDeactivated         );
		// mono_add_internal_call("CImGui::IsItemDeactivatedAfterEdit ", &CImGui::IsItemDeactivatedAfterEdit);
		// mono_add_internal_call("CImGui::IsItemToggledOpen"          , &CImGui::IsItemToggledOpen         );
		// mono_add_internal_call("CImGui::IsAnyItemHovered"           , &CImGui::IsAnyItemHovered          );
		// mono_add_internal_call("CImGui::IsAnyItemActive"	        , &CImGui::IsAnyItemActive           );
		// mono_add_internal_call("CImGui::IsAnyItemFocused"           , &CImGui::IsAnyItemFocused          );
		// mono_add_internal_call("CImGui::GetItemRectMin"             , &CImGui::GetItemRectMin            );
		// mono_add_internal_call("CImGui::GetItemRectMax"             , &CImGui::GetItemRectMax            );
		// mono_add_internal_call("CImGui::GetItemRectSize"            , &CImGui::GetItemRectSize           );
		// mono_add_internal_call("CImGui::SetItemAllowOverlap"        , &CImGui::SetItemAllowOverlap       );
	}
}
*/
