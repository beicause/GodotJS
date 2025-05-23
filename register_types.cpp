#include "register_types.h"

#include "weaver/jsb_weaver.h"

#ifdef TOOLS_ENABLED
#include "weaver-editor/jsb_weaver_editor.h"
#endif

static Ref<ResourceFormatLoaderGodotJSScript> resource_loader_js;
static Ref<ResourceFormatSaverGodotJSScript> resource_saver_js;

void jsb_initialize_module(ModuleInitializationLevel p_level)
{
    if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS)
    {
        GDREGISTER_CLASS(GodotJSScript);
#ifdef TOOLS_ENABLED
        GDREGISTER_CLASS(GodotJSEditorHelper);
        GDREGISTER_CLASS(GodotJSEditorProgress);
#endif

        jsb::impl::GlobalInitialize::init();

        // register javascript language
        GodotJSScriptLanguage* script_language_js = memnew(GodotJSScriptLanguage());
        ScriptServer::register_language(script_language_js);

        resource_loader_js.instantiate();
        ResourceLoader::add_resource_format_loader(resource_loader_js);

        resource_saver_js.instantiate();
        ResourceSaver::add_resource_format_saver(resource_saver_js);

#ifdef TOOLS_ENABLED
        EditorPlugins::add_by_type<GodotJSEditorPlugin>();
#endif
    }
}

void jsb_uninitialize_module(ModuleInitializationLevel p_level)
{
    if (p_level == MODULE_INITIALIZATION_LEVEL_CORE)
    {
        ResourceLoader::remove_resource_format_loader(resource_loader_js);
        resource_loader_js.unref();

        ResourceSaver::remove_resource_format_saver(resource_saver_js);
        resource_saver_js.unref();

        GodotJSScriptLanguage *script_language_js = GodotJSScriptLanguage::get_singleton();
        jsb_check(script_language_js);
        ScriptServer::unregister_language(script_language_js);
        memdelete(script_language_js);
    }
}

#if JSB_GDEXTENSION
extern "C"
{
    GDExtensionBool GDE_EXPORT jsb_gdextension_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization)
    {
        GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(jsb_initialize_module);
        init_obj.register_terminator(jsb_uninitialize_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_CORE);

        return init_obj.init();
    }
}
#endif
