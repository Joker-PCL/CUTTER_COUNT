SET(SOURCES screens/ui_Main.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_1989276256.c
    images/ui_img_home_22_png.c
    fonts/ui_font_CPM300.c
    fonts/ui_font_Kanit35.c
    fonts/ui_font_NotoSansTH60Bold.c)

add_library(ui ${SOURCES})
