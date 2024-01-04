
// NOTE(kv): This is the part the fleury custom layer that matters to me.  It is
// fully functional, and independent of features I haven't absorbed yet.

#include "4coder_fleury_ubiquitous.cpp"
#include "4coder_fleury_lang_list.h"
#include "kv.h"

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

function void
F4_DoFullLex_ASYNC_Inner(Async_Context *actx, Buffer_ID buffer_id)
{
    Application_Links *app = actx->app;
    ProfileScope(app, "[F4] Async Lex");
    Scratch_Block scratch(app);
    
    String_Const_u8 contents = {};
    {
        ProfileBlock(app, "[F4] Async Lex Contents (before mutex)");
        acquire_global_frame_mutex(app);
        ProfileBlock(app, "[F4] Async Lex Contents (after mutex)");
        contents = push_whole_buffer(app, scratch, buffer_id);
        release_global_frame_mutex(app);
    }
    
    i32 limit_factor = 10000;
    
    Token_List list = {};
    b32 canceled = false;
    
    F4_Language *language = F4_LanguageFromBuffer(app, buffer_id);
    
    // NOTE(rjf): Fall back to C++ if we don't have a proper language.
    if(language == 0)
    {
        language = F4_LanguageFromString(S8Lit("cpp"));
    }
    
    if(language != 0)
    {
        void *lexing_state = push_array_zero(scratch, u8, language->lex_state_size);
        language->LexInit(lexing_state, contents);
        for(;;)
        {
            ProfileBlock(app, "[F4] Async Lex Block");
            if(language->LexFullInput(scratch, &list, lexing_state, limit_factor))
            {
                break;
            }
            if(async_check_canceled(actx))
            {
                canceled = true;
                break;
            }
        }
    }
    
    if(!canceled)
    {
        ProfileBlock(app, "[F4] Async Lex Save Results (before mutex)");
        acquire_global_frame_mutex(app);
        ProfileBlock(app, "[F4] Async Lex Save Results (after mutex)");
        Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
        if(scope != 0)
        {
            Base_Allocator *allocator = managed_scope_allocator(app, scope);
            Token_Array *tokens_ptr = scope_attachment(app, scope, attachment_tokens, Token_Array);
            base_free(allocator, tokens_ptr->tokens);
            Token_Array tokens = {};
            tokens.tokens = base_array(allocator, Token, list.total_count);
            tokens.count = list.total_count;
            tokens.max = list.total_count;
            token_fill_memory_from_list(tokens.tokens, &list);
            block_copy_struct(tokens_ptr, &tokens);
        }
        buffer_mark_as_modified(buffer_id);
        release_global_frame_mutex(app);
    }
}

function void
F4_DoFullLex_ASYNC(Async_Context *actx, String_Const_u8 data)
{
    if(data.size == sizeof(Buffer_ID))
    {
        Buffer_ID buffer = *(Buffer_ID*)data.str;
        F4_DoFullLex_ASYNC_Inner(actx, buffer);
    }
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

function BUFFER_EDIT_RANGE_SIG(F4_BufferEditRange)
{
    // buffer_id, new_range, original_size
    ProfileScope(app, "[F4] Buffer Edit Range");

    Range_i64 old_range = Ii64(old_cursor_range.min.pos, old_cursor_range.max.pos);
    
    buffer_shift_fade_ranges(buffer_id, old_range.max, (new_range.max - old_range.max));
    
    {
        code_index_lock();
        Code_Index_File *file = code_index_get_file(buffer_id);
        if (file != 0){
            code_index_shift(file, old_range, range_size(new_range));
        }
        code_index_unlock();
    }
    
    i64 insert_size = range_size(new_range);
    i64 text_shift = replace_range_shift(old_range, insert_size);
    
    Scratch_Block scratch(app);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Async_Task *lex_task_ptr = scope_attachment(app, scope, buffer_lex_task, Async_Task);
    
    Base_Allocator *allocator = managed_scope_allocator(app, scope);
    b32 do_full_relex = false;
    
    // note: invalidate running lex task
    if (async_task_is_running_or_pending(&global_async_system, *lex_task_ptr)){
        async_task_cancel(app, &global_async_system, *lex_task_ptr);
        buffer_unmark_as_modified(buffer_id);
        do_full_relex = true;
        *lex_task_ptr = 0;
    }
    
    Token_Array *ptr = scope_attachment(app, scope, attachment_tokens, Token_Array);
    if (ptr != 0 && ptr->tokens != 0)
    {
        ProfileBlockNamed(app, "attempt resync", profile_attempt_resync);
        
        i64 token_index_first = token_relex_first(ptr, old_range.first, 1);
        i64 token_index_resync_guess =
            token_relex_resync(ptr, old_range.one_past_last, 16);
        
        if (token_index_resync_guess - token_index_first >= 4000){
            do_full_relex = true;
        }
        else
        {
            Token *token_first = ptr->tokens + token_index_first;
            Token *token_resync = ptr->tokens + token_index_resync_guess;
            
            Range_i64 relex_range = Ii64(token_first->pos, token_resync->pos + token_resync->size + text_shift);
            String_Const_u8 partial_text = push_buffer_range(app, scratch, buffer_id, relex_range);
            
            //~ NOTE(rjf): Lex
            F4_Language *language = F4_LanguageFromBuffer(app, buffer_id);
            // NOTE(rjf): Fall back to C++ if we don't have a proper language.
            if(language == 0)
            {
                language = F4_LanguageFromString(S8Lit("cpp"));
            }
            Token_List relex_list = F4_Language_LexFullInput_NoBreaks(app, language, scratch, partial_text);
            //~
            
            if (relex_range.one_past_last < buffer_get_size(app, buffer_id)){
                token_drop_eof(&relex_list);
            }
            
            Token_Relex relex = token_relex(relex_list, relex_range.first - text_shift, ptr->tokens, token_index_first, token_index_resync_guess);
            
            ProfileCloseNow(profile_attempt_resync);
            
            if (relex.successful_resync)
            {
                ProfileBlock(app, "apply resync");
                
                i64 token_index_resync = relex.first_resync_index;
                
                Range_i64 head = Ii64(0, token_index_first);
                Range_i64 replaced = Ii64(token_index_first, token_index_resync);
                Range_i64 tail = Ii64(token_index_resync, ptr->count);
                i64 resynced_count = (token_index_resync_guess + 1) - token_index_resync;
                i64 relexed_count = relex_list.total_count - resynced_count;
                i64 tail_shift = relexed_count - (token_index_resync - token_index_first);
                
                i64 new_tokens_count = ptr->count + tail_shift;
                Token *new_tokens = base_array(allocator, Token, new_tokens_count);
                
                Token *old_tokens = ptr->tokens;
                block_copy_array_shift(new_tokens, old_tokens, head, 0);
                token_fill_memory_from_list(new_tokens + replaced.first, &relex_list, relexed_count);
                for (i64 i = 0, index = replaced.first; i < relexed_count; i += 1, index += 1){
                    new_tokens[index].pos += relex_range.first;
                }
                for (i64 i = tail.first; i < tail.one_past_last; i += 1){
                    old_tokens[i].pos += text_shift;
                }
                block_copy_array_shift(new_tokens, ptr->tokens, tail, tail_shift);
                
                base_free(allocator, ptr->tokens);
                
                ptr->tokens = new_tokens;
                ptr->count = new_tokens_count;
                ptr->max = new_tokens_count;
                
                buffer_mark_as_modified(buffer_id);
            }
            else 
            {
              do_full_relex = true;
            }
        }
    }
    
    if (do_full_relex){
      print_message(app, SCu8("!!!do full relex!!!\n"));
      *lex_task_ptr = async_task_no_dep(&global_async_system, F4_DoFullLex_ASYNC,
                                        make_data_struct(&buffer_id));
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
