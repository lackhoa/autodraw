
// NOTE: experimentation

#include "4coder_fleury_ubiquitous.cpp"
#include "4coder_fleury_lang_list.h"
// BOOKMARK(kv)
#include "4coder_fleury_hooks.cpp"
#include "4coder_fleury_bindings.cpp"

function void
F4_Tick_lite(Application_Links *app, Frame_Info frame_info)
{
  linalloc_clear(&global_frame_arena);
  F4_Index_Tick(app);
  // NOTE(rjf): Default tick stuff from the 4th dimension:
  default_tick(app, frame_info);
}

function DELTA_RULE_SIG(F4_DeltaRule_lite)
{
    Vec2_f32 *velocity = (Vec2_f32*)data;
    if(velocity->x == 0.f)
    {
        velocity->x = 1.f;
        velocity->y = 1.f;
    }
    Smooth_Step step_x = smooth_camera_step(pending.x, velocity->x, 80.f, 1.f/4.f);
    Smooth_Step step_y = smooth_camera_step(pending.y, velocity->y, 80.f, 1.f/4.f);
    *velocity = V2f32(step_x.v, step_y.v);
    return(V2f32(step_x.p, step_y.p));
}

function BUFFER_HOOK_SIG(F4_BeginBuffer_lite)
{
    ProfileScope(app, "[kv/Fleury] Begin Buffer Lite");
    
    Scratch_Block scratch(app);
    b32 treat_as_code = false;
    String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer_id);
    String_Const_u8 buffer_name = push_buffer_base_name(app, scratch, buffer_id);
    
    // NOTE(rjf): Treat as code if the config tells us to.
    if(treat_as_code == false)
    {
        if(file_name.size > 0)
        {
            String_Const_u8 treat_as_code_string = def_get_config_string(scratch, vars_save_string_lit("treat_as_code"));
            String_Const_u8_Array extensions = parse_extension_line_to_extension_list(app, scratch, treat_as_code_string);
            String_Const_u8 ext = string_file_extension(file_name);
            for(i32 i = 0; i < extensions.count; ++i)
            {
                if(string_match(ext, extensions.strings[i]))
                {
                    treat_as_code = true;
                    break;
                }
            }
        }
    }
    
    // NOTE(rjf): Treat as code if we've identified the language of a file.
    if(treat_as_code == false)
    {
        F4_Language *language = F4_LanguageFromBuffer(app, buffer_id);
        if (language)
        {
            treat_as_code = true;
        }
    }
    
    String_ID file_map_id = vars_save_string_lit("keys_file");
    String_ID code_map_id = vars_save_string_lit("keys_code");
    
    Command_Map_ID map_id = (treat_as_code) ? (code_map_id) : (file_map_id);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Command_Map_ID *map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
    *map_id_ptr = map_id;
    
    Line_Ending_Kind setting = guess_line_ending_kind_from_buffer(app, buffer_id);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting, Line_Ending_Kind);
    *eol_setting = setting;
    
    // NOTE(allen): Decide buffer settings
    b32 wrap_lines = false;  // NOTE(kv): vim doesn't allow line wrap (in "vim_begin_buffer_inner")
    b32 use_lexer = false;
    if(treat_as_code)
    {
        use_lexer = true;
    }

    if (use_lexer)
    {
        ProfileBlock(app, "begin buffer kick off lexer");
        Async_Task *lex_task_ptr = scope_attachment(app, scope, buffer_lex_task, Async_Task);
        *lex_task_ptr = async_task_no_dep(&global_async_system, F4_DoFullLex_ASYNC, make_data_struct(&buffer_id));
    }
    
    {
        b32 *wrap_lines_ptr = scope_attachment(app, scope, buffer_wrap_lines, b32);
        *wrap_lines_ptr = wrap_lines;
    }
    
    if (use_lexer)
    {
        buffer_set_layout(app, buffer_id, layout_virt_indent_index_generic);
    }
    else
    {
        if (treat_as_code)
        {
            buffer_set_layout(app, buffer_id, layout_virt_indent_literal_generic);
        }
        else{
            buffer_set_layout(app, buffer_id, layout_generic);
        }
    }

    // no meaning for return
    return(0);
}

void fleury_custom_layer_init(Application_Links *app)
{
    default_framework_init(app);
    global_frame_arena = make_arena(get_base_allocator_system());
    permanent_arena = make_arena(get_base_allocator_system());
    
    // NOTE(rjf): Set up hooks.
    {
        set_all_default_hooks(app);
        //t $          ($  , $                             , $                     );
        set_custom_hook(app, HookID_Tick,                    F4_Tick_lite);
        set_custom_hook(app, HookID_BeginBuffer,             F4_BeginBuffer_lite); // NOTE: just do F4_BeginBuffer, then vim_begin_buffer
        set_custom_hook(app, HookID_DeltaRule,               F4_DeltaRule_lite);
        set_custom_hook(app, HookID_BufferEditRange,         F4_BufferEditRange);  // note(kv): ok
        set_custom_hook_memory_size(app, HookID_DeltaRule, delta_ctx_size(sizeof(Vec2_f32)));
    }
    
    // NOTE(rjf): Set up mapping.
    {
        Thread_Context *tctx = get_thread_context(app);
        mapping_init(tctx, &framework_mapping);
        default_4coder_initialize(app);
        String_ID global_id = vars_save_string_lit("keys_global");
        String_ID file_id   = vars_save_string_lit("keys_file");
        String_ID code_id   = vars_save_string_lit("keys_code");
        setup_essential_mapping(&framework_mapping, global_id, file_id, code_id);
        setup_default_mapping(&framework_mapping, global_id, file_id, code_id);
    }
    
    // NOTE(rjf): Set up custom code index.
    {
        F4_Index_Initialize();
    }
    
    // NOTE(rjf): Register languages.
    {
        F4_RegisterLanguages();
    }
}
