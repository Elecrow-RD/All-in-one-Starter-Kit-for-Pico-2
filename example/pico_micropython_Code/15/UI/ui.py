# This file was generated by SquareLine Studio
# SquareLine Studio version: SquareLine Studio 1.5.1
# LVGL version: 9.1.0
# Project name: lesson04

import lvgl as lv
import ui_images

dispp = lv.display_get_default()
theme = lv.theme_default_init(dispp, lv.palette_main(lv.PALETTE.BLUE), lv.palette_main(lv.PALETTE.RED), False, lv.font_montserrat_14)
dispp.set_theme(theme)

def ui_theme_set(idx):
   return

def SetFlag( obj, flag, value):
    if (value):
        obj.add_flag(flag)
    else:
        obj.remove_flag(flag)
    return

_ui_comp_table = {}
_ui_comp_prev = None
_ui_name_prev = None
_ui_child_prev = None
_ui_comp_table.clear()

def _ui_comp_del_event(e):
    target = e.get_target()
    _ui_comp_table[id(target)].remove()

def ui_comp_get_child(comp, child_name):
    return _ui_comp_table[id(comp)][child_name]

def ui_comp_get_root_from_child(child, compname):
    for component in _ui_comp_table:
        if _ui_comp_table[component]["_CompName"]==compname:
            for part in _ui_comp_table[component]:
                if id(_ui_comp_table[component][part]) == id(child):
                    return _ui_comp_table[component]
    return None
def SetBarProperty(target, id, val):
   if id == 'Value_with_anim': target.set_value(val, lv.ANIM.ON)
   if id == 'Value': target.set_value(val, lv.ANIM.OFF)
   return

def SetPanelProperty(target, id, val):
   if id == 'Position_X': target.set_x(val)
   if id == 'Position_Y': target.set_y(val)
   if id == 'Width': target.set_width(val)
   if id == 'Height': target.set_height(val)
   return

def SetDropdownProperty(target, id, val):
   if id == 'Selected':
      target.set_selected(val)
   return

def SetImageProperty(target, id, val, val2):
   if id == 'Image': target.set_src(val)
   if id == 'Angle': target.set_rotation(val2)
   if id == 'Zoom': target.set_scale(val2)
   return

def SetLabelProperty(target, id, val):
   if id == 'Text': target.set_text(val)
   return

def SetRollerProperty(target, id, val):
   if id == 'Selected':
      target.set_selected(val, lv.ANIM.OFF)
   if id == 'Selected_with_anim':
      target.set_selected(val, lv.ANIM.ON)
   return

def SetSliderProperty(target, id, val):
   if id == 'Value_with_anim': target.set_value(val, lv.ANIM.ON)
   if id == 'Value': target.set_value(val, lv.ANIM.OFF)
   return

def ChangeScreen( src, fademode, speed, delay):
    print("Change screen to: "+str([name for name in globals() if globals()[name] is src]))
    lv.screen_load_anim(src, fademode, speed, delay, False)
    return

def DeleteScreen(src):
    return

def IncrementArc( trg, val):
    trg.set_value(trg.get_value()+val)
    trg.send_event(lv.EVENT.VALUE_CHANGED, None)
    return

def IncrementBar( trg, val, anim):
    trg.set_value(trg.get_value()+val,anim)
    return

def IncrementSlider( trg, val, anim):
    trg.set_value(trg.get_value()+val,anim)
    trg.send_event(lv.EVENT.VALUE_CHANGED, None)
    return

def KeyboardSetTarget( keyboard, textarea):
    keyboard.set_textarea(textarea)
    return

def ModifyFlag( obj, flag, value):
    if (value=="TOGGLE"):
        if ( obj.has_flag(flag) ):
            obj.remove_flag(flag)
        else:
            obj.add_flag(flag)
        return

    if (value=="ADD"):
        obj.add_flag(flag)
    else:
        obj.remove_flag(flag)
    return

def ModifyState( obj, state, value):
    if (value=="TOGGLE"):
        if ( obj.has_state(state) ):
            obj.remove_state(state)
        else:
            obj.add_state(state)
        return

    if (value=="ADD"):
        obj.add_state(state)
    else:
        obj.remove_state(state)
    return

def TextAreaMoveCursor( trg, val):
    if val=="UP" : trg.cursor_up()
    if val=="RIGHT" : trg.cursor_right()
    if val=="DOWN" : trg.cursor_down()
    if val=="LEFT" : trg.cursor_left()
    trg.add_state(lv.STATE.FOCUSED)
    return

def set_opacity(obj, v):
    obj.set_style_opa(v, lv.STATE.DEFAULT|lv.PART.MAIN)
    return

def SetTextValueArc( trg, src, prefix, postfix):
    trg.set_text(prefix+str(src.get_value())+postfix)
    return

def SetTextValueSlider( trg, src, prefix, postfix):
    trg.set_text(prefix+str(src.get_value())+postfix)
    return

def SetTextValueChecked( trg, src, txton, txtoff):
    if src.has_state(lv.STATE.CHECKED):
        trg.set_text(txton)
    else:
        trg.set_text(txtoff)
    return

def StepSpinbox( trg, val):
    if val==1 : trg.increment()
    if val==-1 : trg.decrement()
    trg.send_event(lv.EVENT.VALUE_CHANGED, None)
    return

def SwitchTheme(val):
    ui_theme_set(val)
    return

# COMPONENTS

# COMPONENTS
ui____initial_actions0 = lv.obj()

ui_Screen1 = lv.obj()
SetFlag(ui_Screen1, lv.obj.FLAG.SCROLLABLE, False)

ui_Label1 = lv.label(ui_Screen1)
ui_Label1.set_text("text")
ui_Label1.set_width(lv.SIZE_CONTENT)	# 1
ui_Label1.set_height(lv.SIZE_CONTENT)   # 1
ui_Label1.set_align( lv.ALIGN.CENTER)
ui_Label1.set_style_text_font( lv.font_montserrat_14, lv.PART.MAIN | lv.STATE.DEFAULT )

lv.screen_load(ui_Screen1)
