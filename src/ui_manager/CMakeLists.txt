set(COMPONENT_SRCS 
    ui_manager.c
    generated/main_ui.c
)

set(COMPONENT_ADD_INCLUDEDIRS
    .
    generated
)

idf_component_register(SRCS ${COMPONENT_SRCS}
                       INCLUDE_DIRS ${COMPONENT_ADD_INCLUDEDIRS}) 

                       