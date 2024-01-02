#if !defined(FCODER_LEX_GEN_HAND_WRITTEN)
#define FCODER_LEX_GEN_HAND_WRITTEN

internal u64
lexeme_hash(u64 seed, u8 *ptr, u64 size){
  u64 result = 0;
  for (u64 i = 0; i < size; i += 1, ptr += 1){
    result ^= ((*ptr) ^ result*59) + seed;
  }
  return(result);
}

internal Lexeme_Table_Lookup
lexeme_table_lookup(u64 *hash_array, String_Const_u8 *key_array, 
                    Lexeme_Table_Value *value_array, i32 slot_count, u64 seed,
                    u8 *ptr, u64 size){
  Lexeme_Table_Lookup result = {};
  u64 hash = lexeme_hash(seed, ptr, size);
  u64 comparison_hash = hash | 1;
  i32 first_index = (hash % slot_count);
  i32 index = first_index;
  for (;;){
    if (hash_array[index] == comparison_hash){
      if (string_match(SCu8(ptr, size), key_array[index])){
        result.found_match = true;
        result.base_kind = value_array[index].base_kind;
        result.sub_kind = value_array[index].sub_kind;
        break;
      }
    }
    else if (hash_array[index] == 0){
      break;
    }
    index += 1;
    if (index == slot_count){
      index = 0;
    }
    if (index == first_index){
      break;
    }
  }
  return(result);
}

#endif
u64 jai_main_keys_hash_array[89] = {
  0x0000000000000000,0x0000000000000000,0x0000000000000000,0x9f7e3033bcc9575b,
  0x6e3971fc92b2af83,0x0000000000000000,0x6e3971fc92d0e791,0x6e3971fc9570ece9,
  0x0000000000000000,0xd6570b501ed5cd47,0x0000000000000000,0x6e3971fc924a7865,
  0x9f7e3050bbc4a129,0x80682b7de03e1d9b,0x0000000000000000,0x2fa0e8dffa88b4b7,
  0x0000000000000000,0xae8928363b35164b,0x2fa0e8dffa88a8e7,0x2fa0e8dffa88b729,
  0x0000000000000000,0x0000000000000000,0x0000000000000000,0x80682b7c72ecf6e5,
  0xd6574f79270450f3,0xae8928363b49aa5b,0x9f7e318cbf4847bb,0x2fa0e8dffa88ab5d,
  0x0000000000000000,0x6e3971fc92b17605,0x6e3971fc92e5e299,0x0000000000000000,
  0xae8928363b337477,0x0000000000000000,0xae8928363b49af43,0xd657b137ba1250eb,
  0x0000000000000000,0x6e3971fc924a9a91,0xd657b16956cd1d1f,0x0000000000000000,
  0x3b74b0ddc8631b0f,0xae8928363b34656d,0xae8928363b36f5cf,0x0000000000000000,
  0xae8928363b49a90b,0x0000000000000000,0x9f7e305eeb47fcf9,0x6e3971fc92e5e2a9,
  0x0000000000000000,0x6e3971fc95594915,0x9f7e305eeb48165d,0x0000000000000000,
  0x0000000000000000,0x80682b7c4f16c8f5,0x3b1dd968afc00e6d,0x9f7e3033bcd1825d,
  0xae8928363b355b9f,0x9f7e3195fbff871b,0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x2fa0e8dffa88b7ad,0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x0000000000000000,0xae8928363b485edb,0x2fa0e8dffa88a919,
  0x0000000000000000,0xae8928363b485dd3,0x0000000000000000,0x0000000000000000,
  0x80682b7c4f8edb67,0x0000000000000000,0x13ddb90edc28732b,0x0000000000000000,
  0x0000000000000000,0x3b1d3e4126455aa9,0x0000000000000000,0x0000000000000000,
  0x0000000000000000,0x80682b7de33721d5,0xd657b16956cd1d4b,0xae8928363b485103,
  0x0000000000000000,0x80682b7d2dca2fdf,0x80682b7c4dcfcd1b,0x0000000000000000,
  0xd657b3f060f87b2b,
};
u8 jai_main_keys_key_array_3[] = {0x72,0x65,0x6d,0x6f,0x76,0x65,};
u8 jai_main_keys_key_array_4[] = {0x6e,0x75,0x6c,0x6c,};
u8 jai_main_keys_key_array_6[] = {0x62,0x6f,0x6f,0x6c,};
u8 jai_main_keys_key_array_7[] = {0x74,0x72,0x75,0x65,};
u8 jai_main_keys_key_array_9[] = {0x73,0x69,0x7a,0x65,0x5f,0x6f,0x66,};
u8 jai_main_keys_key_array_11[] = {0x65,0x6c,0x73,0x65,};
u8 jai_main_keys_key_array_12[] = {0x73,0x77,0x69,0x74,0x63,0x68,};
u8 jai_main_keys_key_array_13[] = {0x77,0x68,0x69,0x6c,0x65,};
u8 jai_main_keys_key_array_15[] = {0x78,0x78,};
u8 jai_main_keys_key_array_17[] = {0x69,0x6e,0x74,};
u8 jai_main_keys_key_array_18[] = {0x69,0x66,};
u8 jai_main_keys_key_array_19[] = {0x73,0x38,};
u8 jai_main_keys_key_array_23[] = {0x66,0x61,0x6c,0x73,0x65,};
u8 jai_main_keys_key_array_24[] = {0x61,0x6c,0x69,0x67,0x6e,0x6f,0x66,};
u8 jai_main_keys_key_array_25[] = {0x75,0x33,0x32,};
u8 jai_main_keys_key_array_26[] = {0x64,0x65,0x6c,0x65,0x74,0x65,};
u8 jai_main_keys_key_array_27[] = {0x64,0x6f,};
u8 jai_main_keys_key_array_29[] = {0x67,0x6f,0x74,0x6f,};
u8 jai_main_keys_key_array_30[] = {0x63,0x61,0x73,0x74,};
u8 jai_main_keys_key_array_32[] = {0x41,0x6e,0x79,};
u8 jai_main_keys_key_array_34[] = {0x75,0x31,0x36,};
u8 jai_main_keys_key_array_35[] = {0x64,0x65,0x66,0x61,0x75,0x6c,0x74,};
u8 jai_main_keys_key_array_37[] = {0x65,0x6e,0x75,0x6d,};
u8 jai_main_keys_key_array_38[] = {0x66,0x6c,0x6f,0x61,0x74,0x33,0x32,};
u8 jai_main_keys_key_array_40[] = {0x6f,0x70,0x65,0x72,0x61,0x74,0x6f,0x72,};
u8 jai_main_keys_key_array_41[] = {0x2d,0x2d,0x2d,};
u8 jai_main_keys_key_array_42[] = {0x66,0x6f,0x72,};
u8 jai_main_keys_key_array_44[] = {0x75,0x36,0x34,};
u8 jai_main_keys_key_array_46[] = {0x73,0x74,0x72,0x75,0x63,0x74,};
u8 jai_main_keys_key_array_47[] = {0x63,0x61,0x73,0x65,};
u8 jai_main_keys_key_array_49[] = {0x76,0x6f,0x69,0x64,};
u8 jai_main_keys_key_array_50[] = {0x73,0x74,0x72,0x69,0x6e,0x67,};
u8 jai_main_keys_key_array_53[] = {0x66,0x6c,0x6f,0x61,0x74,};
u8 jai_main_keys_key_array_54[] = {0x69,0x74,0x5f,0x69,0x6e,0x64,0x65,0x78,};
u8 jai_main_keys_key_array_55[] = {0x72,0x65,0x74,0x75,0x72,0x6e,};
u8 jai_main_keys_key_array_56[] = {0x6e,0x65,0x77,};
u8 jai_main_keys_key_array_57[] = {0x69,0x6e,0x6c,0x69,0x6e,0x65,};
u8 jai_main_keys_key_array_61[] = {0x75,0x38,};
u8 jai_main_keys_key_array_66[] = {0x73,0x36,0x34,};
u8 jai_main_keys_key_array_67[] = {0x69,0x74,};
u8 jai_main_keys_key_array_69[] = {0x73,0x33,0x32,};
u8 jai_main_keys_key_array_72[] = {0x62,0x72,0x65,0x61,0x6b,};
u8 jai_main_keys_key_array_74[] = {0x70,0x75,0x73,0x68,0x5f,0x63,0x6f,0x6e,0x74,0x65,0x78,0x74,};
u8 jai_main_keys_key_array_77[] = {0x63,0x6f,0x6e,0x74,0x69,0x6e,0x75,0x65,};
u8 jai_main_keys_key_array_81[] = {0x75,0x73,0x69,0x6e,0x67,};
u8 jai_main_keys_key_array_82[] = {0x66,0x6c,0x6f,0x61,0x74,0x36,0x34,};
u8 jai_main_keys_key_array_83[] = {0x73,0x31,0x36,};
u8 jai_main_keys_key_array_85[] = {0x75,0x6e,0x69,0x6f,0x6e,};
u8 jai_main_keys_key_array_86[] = {0x64,0x65,0x66,0x65,0x72,};
u8 jai_main_keys_key_array_88[] = {0x63,0x6f,0x6e,0x74,0x65,0x78,0x74,};
String_Const_u8 jai_main_keys_key_array[89] = {
  {0, 0},
  {0, 0},
  {0, 0},
  {jai_main_keys_key_array_3, 6},
  {jai_main_keys_key_array_4, 4},
  {0, 0},
  {jai_main_keys_key_array_6, 4},
  {jai_main_keys_key_array_7, 4},
  {0, 0},
  {jai_main_keys_key_array_9, 7},
  {0, 0},
  {jai_main_keys_key_array_11, 4},
  {jai_main_keys_key_array_12, 6},
  {jai_main_keys_key_array_13, 5},
  {0, 0},
  {jai_main_keys_key_array_15, 2},
  {0, 0},
  {jai_main_keys_key_array_17, 3},
  {jai_main_keys_key_array_18, 2},
  {jai_main_keys_key_array_19, 2},
  {0, 0},
  {0, 0},
  {0, 0},
  {jai_main_keys_key_array_23, 5},
  {jai_main_keys_key_array_24, 7},
  {jai_main_keys_key_array_25, 3},
  {jai_main_keys_key_array_26, 6},
  {jai_main_keys_key_array_27, 2},
  {0, 0},
  {jai_main_keys_key_array_29, 4},
  {jai_main_keys_key_array_30, 4},
  {0, 0},
  {jai_main_keys_key_array_32, 3},
  {0, 0},
  {jai_main_keys_key_array_34, 3},
  {jai_main_keys_key_array_35, 7},
  {0, 0},
  {jai_main_keys_key_array_37, 4},
  {jai_main_keys_key_array_38, 7},
  {0, 0},
  {jai_main_keys_key_array_40, 8},
  {jai_main_keys_key_array_41, 3},
  {jai_main_keys_key_array_42, 3},
  {0, 0},
  {jai_main_keys_key_array_44, 3},
  {0, 0},
  {jai_main_keys_key_array_46, 6},
  {jai_main_keys_key_array_47, 4},
  {0, 0},
  {jai_main_keys_key_array_49, 4},
  {jai_main_keys_key_array_50, 6},
  {0, 0},
  {0, 0},
  {jai_main_keys_key_array_53, 5},
  {jai_main_keys_key_array_54, 8},
  {jai_main_keys_key_array_55, 6},
  {jai_main_keys_key_array_56, 3},
  {jai_main_keys_key_array_57, 6},
  {0, 0},
  {0, 0},
  {0, 0},
  {jai_main_keys_key_array_61, 2},
  {0, 0},
  {0, 0},
  {0, 0},
  {0, 0},
  {jai_main_keys_key_array_66, 3},
  {jai_main_keys_key_array_67, 2},
  {0, 0},
  {jai_main_keys_key_array_69, 3},
  {0, 0},
  {0, 0},
  {jai_main_keys_key_array_72, 5},
  {0, 0},
  {jai_main_keys_key_array_74, 12},
  {0, 0},
  {0, 0},
  {jai_main_keys_key_array_77, 8},
  {0, 0},
  {0, 0},
  {0, 0},
  {jai_main_keys_key_array_81, 5},
  {jai_main_keys_key_array_82, 7},
  {jai_main_keys_key_array_83, 3},
  {0, 0},
  {jai_main_keys_key_array_85, 5},
  {jai_main_keys_key_array_86, 5},
  {0, 0},
  {jai_main_keys_key_array_88, 7},
};
Lexeme_Table_Value jai_main_keys_value_array[89] = {
  {0, 0},
  {0, 0},
  {0, 0},
  {4, TokenJaiKind_Remove},
  {4, TokenJaiKind_Null},
  {0, 0},
  {4, TokenJaiKind_Bool},
  {8, TokenJaiKind_LiteralTrue},
  {0, 0},
  {4, TokenJaiKind_SizeOf},
  {0, 0},
  {4, TokenJaiKind_Else},
  {4, TokenJaiKind_Switch},
  {4, TokenJaiKind_While},
  {0, 0},
  {4, TokenJaiKind_Autocast},
  {0, 0},
  {4, TokenJaiKind_Int},
  {4, TokenJaiKind_If},
  {4, TokenJaiKind_S8},
  {0, 0},
  {0, 0},
  {0, 0},
  {8, TokenJaiKind_LiteralFalse},
  {4, TokenJaiKind_AlignOf},
  {4, TokenJaiKind_U32},
  {4, TokenJaiKind_Delete},
  {4, TokenJaiKind_Do},
  {0, 0},
  {4, TokenJaiKind_Goto},
  {4, TokenJaiKind_Cast},
  {0, 0},
  {4, TokenJaiKind_Any},
  {0, 0},
  {4, TokenJaiKind_U16},
  {4, TokenJaiKind_Default},
  {0, 0},
  {4, TokenJaiKind_Enum},
  {4, TokenJaiKind_Float32},
  {0, 0},
  {4, TokenJaiKind_Operator},
  {4, TokenJaiKind_Uninitialized},
  {4, TokenJaiKind_For},
  {0, 0},
  {4, TokenJaiKind_U64},
  {0, 0},
  {4, TokenJaiKind_Struct},
  {4, TokenJaiKind_Case},
  {0, 0},
  {4, TokenJaiKind_Void},
  {4, TokenJaiKind_String},
  {0, 0},
  {0, 0},
  {4, TokenJaiKind_Float},
  {4, TokenJaiKind_ItIndex},
  {4, TokenJaiKind_Return},
  {4, TokenJaiKind_New},
  {4, TokenJaiKind_Inline},
  {0, 0},
  {0, 0},
  {0, 0},
  {4, TokenJaiKind_U8},
  {0, 0},
  {0, 0},
  {0, 0},
  {0, 0},
  {4, TokenJaiKind_S64},
  {4, TokenJaiKind_It},
  {0, 0},
  {4, TokenJaiKind_S32},
  {0, 0},
  {0, 0},
  {4, TokenJaiKind_Break},
  {0, 0},
  {4, TokenJaiKind_PushContext},
  {0, 0},
  {0, 0},
  {4, TokenJaiKind_Continue},
  {0, 0},
  {0, 0},
  {0, 0},
  {4, TokenJaiKind_Using},
  {4, TokenJaiKind_Float64},
  {4, TokenJaiKind_S16},
  {0, 0},
  {4, TokenJaiKind_Union},
  {4, TokenJaiKind_Defer},
  {0, 0},
  {4, TokenJaiKind_Context},
};
i32 jai_main_keys_slot_count = 89;
u64 jai_main_keys_seed = 0x8714154b043ec5b6;
u64 jai_pp_directives_hash_array[26] = {
  0x0000000000000000,0xe139c49dacb803d3,0x0000000000000000,0x662df6e4e9c085ed,
  0x0000000000000000,0xe139c49dd5f9a70b,0x662df6e4c477190b,0x0000000000000000,
  0x662df6e1b7e7587f,0xe139c49dad64de3b,0x4d763e5c70bb9917,0x27b9700a9e2b03bf,
  0x0000000000000000,0x27b9700a9e4621db,0x0000000000000000,0x0000000000000000,
  0x27b9700a9e462c6f,0xe139c49dd0dc65bd,0x0000000000000000,0x4d766c1da7461917,
  0x662df6e71b441c8b,0x0000000000000000,0x0000000000000000,0xe139c49da383850b,
  0xab2ae2bffd76667b,0x27b9700a9e24cd39,
};
u8 jai_pp_directives_key_array_1[] = {0x65,0x72,0x72,0x6f,0x72,};
u8 jai_pp_directives_key_array_3[] = {0x69,0x6d,0x70,0x6f,0x72,0x74,};
u8 jai_pp_directives_key_array_5[] = {0x75,0x6e,0x64,0x65,0x66,};
u8 jai_pp_directives_key_array_6[] = {0x69,0x66,0x6e,0x64,0x65,0x66,};
u8 jai_pp_directives_key_array_8[] = {0x64,0x65,0x66,0x69,0x6e,0x65,};
u8 jai_pp_directives_key_array_9[] = {0x65,0x6e,0x64,0x69,0x66,};
u8 jai_pp_directives_key_array_10[] = {0x69,0x6e,0x63,0x6c,0x75,0x64,0x65,};
u8 jai_pp_directives_key_array_11[] = {0x6c,0x69,0x6e,0x65,};
u8 jai_pp_directives_key_array_13[] = {0x65,0x6c,0x69,0x66,};
u8 jai_pp_directives_key_array_16[] = {0x65,0x6c,0x73,0x65,};
u8 jai_pp_directives_key_array_17[] = {0x75,0x73,0x69,0x6e,0x67,};
u8 jai_pp_directives_key_array_19[] = {0x76,0x65,0x72,0x73,0x69,0x6f,0x6e,};
u8 jai_pp_directives_key_array_20[] = {0x70,0x72,0x61,0x67,0x6d,0x61,};
u8 jai_pp_directives_key_array_23[] = {0x69,0x66,0x64,0x65,0x66,};
u8 jai_pp_directives_key_array_24[] = {0x69,0x66,};
u8 jai_pp_directives_key_array_25[] = {0x6c,0x6f,0x61,0x64,};
String_Const_u8 jai_pp_directives_key_array[26] = {
  {0, 0},
  {jai_pp_directives_key_array_1, 5},
  {0, 0},
  {jai_pp_directives_key_array_3, 6},
  {0, 0},
  {jai_pp_directives_key_array_5, 5},
  {jai_pp_directives_key_array_6, 6},
  {0, 0},
  {jai_pp_directives_key_array_8, 6},
  {jai_pp_directives_key_array_9, 5},
  {jai_pp_directives_key_array_10, 7},
  {jai_pp_directives_key_array_11, 4},
  {0, 0},
  {jai_pp_directives_key_array_13, 4},
  {0, 0},
  {0, 0},
  {jai_pp_directives_key_array_16, 4},
  {jai_pp_directives_key_array_17, 5},
  {0, 0},
  {jai_pp_directives_key_array_19, 7},
  {jai_pp_directives_key_array_20, 6},
  {0, 0},
  {0, 0},
  {jai_pp_directives_key_array_23, 5},
  {jai_pp_directives_key_array_24, 2},
  {jai_pp_directives_key_array_25, 4},
};
Lexeme_Table_Value jai_pp_directives_value_array[26] = {
  {0, 0},
  {6, TokenJaiKind_PPError},
  {0, 0},
  {6, TokenJaiKind_PPImport},
  {0, 0},
  {6, TokenJaiKind_PPUndef},
  {6, TokenJaiKind_PPIfNDef},
  {0, 0},
  {6, TokenJaiKind_PPDefine},
  {6, TokenJaiKind_PPEndIf},
  {6, TokenJaiKind_PPInclude},
  {6, TokenJaiKind_PPLine},
  {0, 0},
  {6, TokenJaiKind_PPElIf},
  {0, 0},
  {0, 0},
  {6, TokenJaiKind_PPElse},
  {6, TokenJaiKind_PPUsing},
  {0, 0},
  {6, TokenJaiKind_PPVersion},
  {6, TokenJaiKind_PPPragma},
  {0, 0},
  {0, 0},
  {6, TokenJaiKind_PPIfDef},
  {6, TokenJaiKind_PPIf},
  {6, TokenJaiKind_PPLoad},
};
i32 jai_pp_directives_slot_count = 26;
u64 jai_pp_directives_seed = 0x9ec3e224b8a17329;
u64 jai_pp_keys_hash_array[2] = {
  0xacc876e58b855ebb,0x0000000000000000,
};
u8 jai_pp_keys_key_array_0[] = {0x64,0x65,0x66,0x69,0x6e,0x65,0x64,};
String_Const_u8 jai_pp_keys_key_array[2] = {
  {jai_pp_keys_key_array_0, 7},
  {0, 0},
};
Lexeme_Table_Value jai_pp_keys_value_array[2] = {
  {4, TokenJaiKind_PPDefined},
  {0, 0},
};
i32 jai_pp_keys_slot_count = 2;
u64 jai_pp_keys_seed = 0xb18da329935d825c;
struct Lex_State_Jai{
  u32 flags_ZF0;
  u32 flags_KF0;
  u16 flags_ZB0;
  u8 *base;
  u8 *delim_first;
  u8 *delim_one_past_last;
  u8 *emit_ptr;
  u8 *ptr;
  u8 *opl_ptr;
};
internal void
lex_full_input_jai_init(Lex_State_Jai *state_ptr, String_Const_u8 input){
  state_ptr->flags_ZF0 = 0;
  state_ptr->flags_KF0 = 0;
  state_ptr->flags_ZB0 = 0;
  state_ptr->base = input.str;
  state_ptr->delim_first = input.str;
  state_ptr->delim_one_past_last = input.str;
  state_ptr->emit_ptr = input.str;
  state_ptr->ptr = input.str;
  state_ptr->opl_ptr = input.str + input.size;
}
internal b32
lex_full_input_jai_breaks(Arena *arena, Token_List *list, Lex_State_Jai *state_ptr, u64 max){
  b32 result = false;
  u64 emit_counter = 0;
  Lex_State_Jai state;
  block_copy_struct(&state, state_ptr);
  {
state_label_1: // root
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_EOF;
            token.kind = 0;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        result = true;
        goto end;
      }
    }
    switch (*state.ptr){
      case 0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:
      case 0x07:case 0x08:case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:
      case 0x13:case 0x14:case 0x15:case 0x16:case 0x17:case 0x18:case 0x19:
      case 0x1a:case 0x1b:case 0x1c:case 0x1d:case 0x1e:case 0x1f:case 0x40:
      case 0x60:case 0x7f:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x09:case 0x0b:case 0x0c:case 0x0d:case 0x20:
      {
        if ((HasFlag(state.flags_KF0, 0x2))){
          state.ptr += 1;
          goto state_label_4; // error_body
        }
        state.ptr += 1;
        goto state_label_3; // whitespace
      }break;
      case 0x0a:
      {
        state.ptr += 1;
        state.flags_ZB0 &= ~(0x1);
        state.flags_KF0 &= ~(0x1);
        state.flags_KF0 &= ~(0x2);
        goto state_label_3; // whitespace
      }break;
      case 0x21:
      {
        state.ptr += 1;
        goto state_label_58; // op stage
      }break;
      case 0x22:
      {
        if ((HasFlag(state.flags_KF0, 0x1))){
          state.ptr += 1;
          goto state_label_26; // include_quotes
        }
        state.ptr += 1;
        goto state_label_32; // string
      }break;
      case 0x23:
      {
        if ((!HasFlag(state.flags_ZB0, 0x1))){
          state.ptr += 1;
          goto state_label_23; // pp_directive_whitespace
        }
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      default:
      {
        state.ptr += 1;
        goto state_label_2; // identifier
      }break;
      case 0x25:
      {
        state.ptr += 1;
        goto state_label_63; // op stage
      }break;
      case 0x26:
      {
        state.ptr += 1;
        goto state_label_59; // op stage
      }break;
      case 0x27:
      {
        state.ptr += 1;
        state.flags_ZF0 |= 0x40;
        goto state_label_32; // string
      }break;
      case 0x28:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_ParenOp;
            token.kind = 13;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x29:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_ParenCl;
            token.kind = 14;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x2a:
      {
        state.ptr += 1;
        goto state_label_62; // op stage
      }break;
      case 0x2b:
      {
        state.ptr += 1;
        goto state_label_61; // op stage
      }break;
      case 0x2c:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Comma;
            token.kind = 15;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x2d:
      {
        state.ptr += 1;
        goto state_label_54; // op stage
      }break;
      case 0x2e:
      {
        state.ptr += 1;
        goto state_label_6; // operator_or_fnumber_dot
      }break;
      case 0x2f:
      {
        state.ptr += 1;
        goto state_label_7; // operator_or_comment_slash
      }break;
      case 0x30:
      {
        state.ptr += 1;
        goto state_label_9; // znumber
      }break;
      case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:case 0x37:
      case 0x38:case 0x39:
      {
        state.ptr += 1;
        goto state_label_8; // number
      }break;
      case 0x3a:
      {
        state.ptr += 1;
        goto state_label_53; // op stage
      }break;
      case 0x3b:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Semicolon;
            token.kind = 15;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3c:
      {
        if ((!HasFlag(state.flags_KF0, 0x1))){
          state.ptr += 1;
          goto state_label_56; // op stage
        }
        state.ptr += 1;
        goto state_label_25; // include_pointy
      }break;
      case 0x3d:
      {
        state.ptr += 1;
        goto state_label_55; // op stage
      }break;
      case 0x3e:
      {
        state.ptr += 1;
        goto state_label_57; // op stage
      }break;
      case 0x3f:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Ternary;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x4c:
      {
        state.ptr += 1;
        state.flags_ZF0 |= 0x4;
        goto state_label_27; // pre_L
      }break;
      case 0x52:
      {
        state.ptr += 1;
        goto state_label_31; // pre_R
      }break;
      case 0x55:
      {
        state.ptr += 1;
        state.flags_ZF0 |= 0x20;
        goto state_label_29; // pre_U
      }break;
      case 0x5b:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_BrackOp;
            token.kind = 13;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x5c:
      {
        state.ptr += 1;
        goto state_label_5; // backslash
      }break;
      case 0x5d:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_BrackCl;
            token.kind = 14;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x5e:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Xor;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x75:
      {
        state.ptr += 1;
        state.flags_ZF0 |= 0x10;
        goto state_label_28; // pre_u
      }break;
      case 0x7b:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_BraceOp;
            token.kind = 11;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x7c:
      {
        state.ptr += 1;
        goto state_label_60; // op stage
      }break;
      case 0x7d:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_BraceCl;
            token.kind = 12;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x7e:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Tilde;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_2: // identifier
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZB0, 0x1)){
              Lexeme_Table_Lookup lookup = lexeme_table_lookup(jai_pp_keys_hash_array, jai_pp_keys_key_array, jai_pp_keys_value_array, jai_pp_keys_slot_count, jai_pp_keys_seed, state.emit_ptr, token.size);
              if (lookup.found_match){
                token.kind = lookup.base_kind;
                token.sub_kind = lookup.sub_kind;
                break;
              }
            }
            Lexeme_Table_Lookup lookup = lexeme_table_lookup(jai_main_keys_hash_array, jai_main_keys_key_array, jai_main_keys_value_array, jai_main_keys_slot_count, jai_main_keys_seed, state.emit_ptr, token.size);
            if (lookup.found_match){
              token.kind = lookup.base_kind;
              token.sub_kind = lookup.sub_kind;
              break;
            }
            token.sub_kind = TokenJaiKind_Identifier;
            token.kind = 6;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      case 0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:
      case 0x07:case 0x08:case 0x09:case 0x0a:case 0x0b:case 0x0c:case 0x0d:
      case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:case 0x13:case 0x14:
      case 0x15:case 0x16:case 0x17:case 0x18:case 0x19:case 0x1a:case 0x1b:
      case 0x1c:case 0x1d:case 0x1e:case 0x1f:case 0x20:case 0x21:case 0x22:
      case 0x23:case 0x25:case 0x26:case 0x27:case 0x28:case 0x29:case 0x2a:
      case 0x2b:case 0x2c:case 0x2d:case 0x2e:case 0x2f:case 0x3a:case 0x3b:
      case 0x3c:case 0x3d:case 0x3e:case 0x3f:case 0x40:case 0x5b:case 0x5c:
      case 0x5d:case 0x5e:case 0x60:case 0x7b:case 0x7c:case 0x7d:case 0x7e:
      case 0x7f:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZB0, 0x1)){
              Lexeme_Table_Lookup lookup = lexeme_table_lookup(jai_pp_keys_hash_array, jai_pp_keys_key_array, jai_pp_keys_value_array, jai_pp_keys_slot_count, jai_pp_keys_seed, state.emit_ptr, token.size);
              if (lookup.found_match){
                token.kind = lookup.base_kind;
                token.sub_kind = lookup.sub_kind;
                break;
              }
            }
            Lexeme_Table_Lookup lookup = lexeme_table_lookup(jai_main_keys_hash_array, jai_main_keys_key_array, jai_main_keys_value_array, jai_main_keys_slot_count, jai_main_keys_seed, state.emit_ptr, token.size);
            if (lookup.found_match){
              token.kind = lookup.base_kind;
              token.sub_kind = lookup.sub_kind;
              break;
            }
            token.sub_kind = TokenJaiKind_Identifier;
            token.kind = 6;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      default:
      {
        state.ptr += 1;
        goto state_label_2; // identifier
      }break;
    }
  }
  {
state_label_3: // whitespace
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Whitespace;
            token.kind = 1;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Whitespace;
            token.kind = 1;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x09:case 0x0b:case 0x0c:case 0x0d:case 0x20:
      {
        state.ptr += 1;
        goto state_label_3; // whitespace
      }break;
      case 0x0a:
      {
        state.ptr += 1;
        state.flags_ZB0 &= ~(0x1);
        state.flags_KF0 &= ~(0x1);
        state.flags_KF0 &= ~(0x2);
        goto state_label_3; // whitespace
      }break;
    }
  }
  {
state_label_4: // error_body
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_PPErrorMessage;
            token.kind = 10;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        state.ptr += 1;
        goto state_label_4; // error_body
      }break;
      case 0x0a:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_PPErrorMessage;
            token.kind = 10;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_5: // backslash
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Backslash;
            token.kind = 1;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Backslash;
            token.kind = 1;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x0a:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Backslash;
            token.kind = 1;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x0d:
      {
        state.ptr += 1;
        goto state_label_5; // backslash
      }break;
    }
  }
  {
state_label_6: // operator_or_fnumber_dot
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Dot;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Dot;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x2a:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_DotStar;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x2e:
      {
        state.ptr += 1;
        goto state_label_66; // op stage
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:
      {
        state.ptr += 1;
        goto state_label_10; // fnumber_decimal
      }break;
    }
  }
  {
state_label_7: // operator_or_comment_slash
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Div;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Div;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x2a:
      {
        state.ptr += 1;
        goto state_label_49; // comment_block
      }break;
      case 0x2f:
      {
        state.ptr += 1;
        goto state_label_51; // comment_line
      }break;
      case 0x3d:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_DivEq;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_8: // number
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralInteger;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralInteger;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x2e:
      {
        state.ptr += 1;
        goto state_label_10; // fnumber_decimal
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:
      {
        state.ptr += 1;
        goto state_label_8; // number
      }break;
      case 0x45:case 0x65:
      {
        state.ptr += 1;
        goto state_label_11; // fnumber_exponent
      }break;
      case 0x4c:
      {
        state.ptr += 1;
        goto state_label_18; // L_number
      }break;
      case 0x55:case 0x75:
      {
        state.ptr += 1;
        goto state_label_17; // U_number
      }break;
      case 0x6c:
      {
        state.ptr += 1;
        goto state_label_20; // l_number
      }break;
    }
  }
  {
state_label_9: // znumber
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralInteger;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralInteger;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x2e:
      {
        state.ptr += 1;
        goto state_label_10; // fnumber_decimal
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:
      {
        state.ptr += 1;
        state.flags_ZF0 |= 0x2;
        goto state_label_16; // number_oct
      }break;
      case 0x45:case 0x65:
      {
        state.ptr += 1;
        goto state_label_11; // fnumber_exponent
      }break;
      case 0x4c:
      {
        state.ptr += 1;
        goto state_label_18; // L_number
      }break;
      case 0x55:case 0x75:
      {
        state.ptr += 1;
        goto state_label_17; // U_number
      }break;
      case 0x58:case 0x78:
      {
        state.ptr += 1;
        state.flags_ZF0 |= 0x1;
        goto state_label_14; // number_hex_first
      }break;
      case 0x6c:
      {
        state.ptr += 1;
        goto state_label_20; // l_number
      }break;
    }
  }
  {
state_label_10: // fnumber_decimal
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat64;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat64;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:
      {
        state.ptr += 1;
        goto state_label_10; // fnumber_decimal
      }break;
      case 0x45:case 0x65:
      {
        state.ptr += 1;
        goto state_label_11; // fnumber_exponent
      }break;
      case 0x46:case 0x66:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat32;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x4c:case 0x6c:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat64;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_11: // fnumber_exponent
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat64;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat64;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x2b:case 0x2d:
      {
        state.ptr += 1;
        goto state_label_12; // fnumber_exponent_sign
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:
      {
        state.ptr += 1;
        goto state_label_13; // fnumber_exponent_digits
      }break;
      case 0x46:case 0x66:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat32;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x4c:case 0x6c:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat64;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_12: // fnumber_exponent_sign
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat64;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat64;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:
      {
        state.ptr += 1;
        goto state_label_13; // fnumber_exponent_digits
      }break;
      case 0x46:case 0x66:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat32;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x4c:case 0x6c:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat64;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_13: // fnumber_exponent_digits
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat64;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat64;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:
      {
        state.ptr += 1;
        goto state_label_13; // fnumber_exponent_digits
      }break;
      case 0x46:case 0x66:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat32;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x4c:case 0x6c:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralFloat64;
            token.kind = 9;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_14: // number_hex_first
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
      case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
      case 0x66:
      {
        state.ptr += 1;
        goto state_label_15; // number_hex
      }break;
    }
  }
  {
state_label_15: // number_hex
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralIntegerHex;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralIntegerHex;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
      case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
      case 0x66:
      {
        state.ptr += 1;
        goto state_label_15; // number_hex
      }break;
      case 0x4c:
      {
        state.ptr += 1;
        goto state_label_18; // L_number
      }break;
      case 0x55:case 0x75:
      {
        state.ptr += 1;
        goto state_label_17; // U_number
      }break;
      case 0x6c:
      {
        state.ptr += 1;
        goto state_label_20; // l_number
      }break;
    }
  }
  {
state_label_16: // number_oct
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralIntegerOct;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LiteralIntegerOct;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:
      {
        state.ptr += 1;
        state.flags_ZF0 |= 0x2;
        goto state_label_16; // number_oct
      }break;
      case 0x4c:
      {
        state.ptr += 1;
        goto state_label_18; // L_number
      }break;
      case 0x55:case 0x75:
      {
        state.ptr += 1;
        goto state_label_17; // U_number
      }break;
      case 0x6c:
      {
        state.ptr += 1;
        goto state_label_20; // l_number
      }break;
    }
  }
  {
state_label_17: // U_number
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexU;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctU;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerU;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexU;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctU;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerU;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x4c:
      {
        state.ptr += 1;
        goto state_label_19; // UL_number
      }break;
      case 0x6c:
      {
        state.ptr += 1;
        goto state_label_21; // Ul_number
      }break;
    }
  }
  {
state_label_18: // L_number
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x4c:
      {
        state.ptr += 1;
        goto state_label_22; // LL_number
      }break;
      case 0x55:case 0x75:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexUL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctUL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerUL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_19: // UL_number
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexUL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctUL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerUL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexUL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctUL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerUL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x4c:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexULL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctULL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerULL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_20: // l_number
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x55:case 0x75:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexUL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctUL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerUL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x6c:
      {
        state.ptr += 1;
        goto state_label_22; // LL_number
      }break;
    }
  }
  {
state_label_21: // Ul_number
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexUL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctUL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerUL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexUL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctUL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerUL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x6c:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexULL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctULL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerULL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_22: // LL_number
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexLL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctLL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerLL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexLL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctLL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerLL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x55:case 0x75:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x1)){
              token.sub_kind = TokenJaiKind_LiteralIntegerHexULL;
              token.kind = 8;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x2)){
              token.sub_kind = TokenJaiKind_LiteralIntegerOctULL;
              token.kind = 8;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralIntegerULL;
            token.kind = 8;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_23: // pp_directive_whitespace
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x09:case 0x0b:case 0x0c:case 0x20:
      {
        state.ptr += 1;
        goto state_label_23; // pp_directive_whitespace
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
      case 0x45:case 0x46:case 0x47:case 0x48:case 0x49:case 0x4a:case 0x4b:
      case 0x4c:case 0x4d:case 0x4e:case 0x4f:case 0x50:case 0x51:case 0x52:
      case 0x53:case 0x54:case 0x55:case 0x56:case 0x57:case 0x58:case 0x59:
      case 0x5a:case 0x5f:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
      case 0x66:case 0x67:case 0x68:case 0x69:case 0x6a:case 0x6b:case 0x6c:
      case 0x6d:case 0x6e:case 0x6f:case 0x70:case 0x71:case 0x72:case 0x73:
      case 0x74:case 0x75:case 0x76:case 0x77:case 0x78:case 0x79:case 0x7a:
      {
        state.delim_first = state.ptr;
        state.flags_ZB0 |= 0x1;
        state.ptr += 1;
        goto state_label_24; // pp_directive
      }break;
    }
  }
  {
state_label_24: // pp_directive
    if (state.ptr == state.opl_ptr){
      if ((true)){
        state.delim_one_past_last = state.ptr;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            Lexeme_Table_Lookup lookup = lexeme_table_lookup(jai_pp_directives_hash_array, jai_pp_directives_key_array, jai_pp_directives_value_array, jai_pp_directives_slot_count, jai_pp_directives_seed, state.delim_first, (state.delim_one_past_last - state.delim_first));
            if (lookup.found_match){
              token.kind = lookup.base_kind;
              token.sub_kind = lookup.sub_kind;
              break;
            }
            token.sub_kind = TokenJaiKind_PPUnknown;
            token.kind = 2;
          }while(0);
          switch (token.sub_kind){
            case TokenJaiKind_PPInclude:
            {
              state.flags_KF0 |= 0x1;
            }break;
            case TokenJaiKind_PPError:
            {
              state.flags_KF0 |= 0x2;
            }break;
          }
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        state.delim_one_past_last = state.ptr;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            Lexeme_Table_Lookup lookup = lexeme_table_lookup(jai_pp_directives_hash_array, jai_pp_directives_key_array, jai_pp_directives_value_array, jai_pp_directives_slot_count, jai_pp_directives_seed, state.delim_first, (state.delim_one_past_last - state.delim_first));
            if (lookup.found_match){
              token.kind = lookup.base_kind;
              token.sub_kind = lookup.sub_kind;
              break;
            }
            token.sub_kind = TokenJaiKind_PPUnknown;
            token.kind = 2;
          }while(0);
          switch (token.sub_kind){
            case TokenJaiKind_PPInclude:
            {
              state.flags_KF0 |= 0x1;
            }break;
            case TokenJaiKind_PPError:
            {
              state.flags_KF0 |= 0x2;
            }break;
          }
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
      case 0x45:case 0x46:case 0x47:case 0x48:case 0x49:case 0x4a:case 0x4b:
      case 0x4c:case 0x4d:case 0x4e:case 0x4f:case 0x50:case 0x51:case 0x52:
      case 0x53:case 0x54:case 0x55:case 0x56:case 0x57:case 0x58:case 0x59:
      case 0x5a:case 0x5f:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
      case 0x66:case 0x67:case 0x68:case 0x69:case 0x6a:case 0x6b:case 0x6c:
      case 0x6d:case 0x6e:case 0x6f:case 0x70:case 0x71:case 0x72:case 0x73:
      case 0x74:case 0x75:case 0x76:case 0x77:case 0x78:case 0x79:case 0x7a:
      {
        state.ptr += 1;
        goto state_label_24; // pp_directive
      }break;
    }
  }
  {
state_label_25: // include_pointy
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x20:case 0x2e:case 0x2f:case 0x30:case 0x31:case 0x32:case 0x33:
      case 0x34:case 0x35:case 0x36:case 0x37:case 0x38:case 0x39:case 0x41:
      case 0x42:case 0x43:case 0x44:case 0x45:case 0x46:case 0x47:case 0x48:
      case 0x49:case 0x4a:case 0x4b:case 0x4c:case 0x4d:case 0x4e:case 0x4f:
      case 0x50:case 0x51:case 0x52:case 0x53:case 0x54:case 0x55:case 0x56:
      case 0x57:case 0x58:case 0x59:case 0x5a:case 0x5c:case 0x5f:case 0x61:
      case 0x62:case 0x63:case 0x64:case 0x65:case 0x66:case 0x67:case 0x68:
      case 0x69:case 0x6a:case 0x6b:case 0x6c:case 0x6d:case 0x6e:case 0x6f:
      case 0x70:case 0x71:case 0x72:case 0x73:case 0x74:case 0x75:case 0x76:
      case 0x77:case 0x78:case 0x79:case 0x7a:
      {
        state.ptr += 1;
        goto state_label_25; // include_pointy
      }break;
      case 0x3e:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_PPIncludeFile;
            token.kind = 10;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_26: // include_quotes
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x20:case 0x2e:case 0x2f:case 0x30:case 0x31:case 0x32:case 0x33:
      case 0x34:case 0x35:case 0x36:case 0x37:case 0x38:case 0x39:case 0x41:
      case 0x42:case 0x43:case 0x44:case 0x45:case 0x46:case 0x47:case 0x48:
      case 0x49:case 0x4a:case 0x4b:case 0x4c:case 0x4d:case 0x4e:case 0x4f:
      case 0x50:case 0x51:case 0x52:case 0x53:case 0x54:case 0x55:case 0x56:
      case 0x57:case 0x58:case 0x59:case 0x5a:case 0x5c:case 0x5f:case 0x61:
      case 0x62:case 0x63:case 0x64:case 0x65:case 0x66:case 0x67:case 0x68:
      case 0x69:case 0x6a:case 0x6b:case 0x6c:case 0x6d:case 0x6e:case 0x6f:
      case 0x70:case 0x71:case 0x72:case 0x73:case 0x74:case 0x75:case 0x76:
      case 0x77:case 0x78:case 0x79:case 0x7a:
      {
        state.ptr += 1;
        goto state_label_26; // include_quotes
      }break;
      case 0x22:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_PPIncludeFile;
            token.kind = 10;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_27: // pre_L
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_2; // identifier
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_2; // identifier
      }break;
      case 0x22:
      {
        state.ptr += 1;
        goto state_label_32; // string
      }break;
      case 0x52:
      {
        state.ptr += 1;
        goto state_label_31; // pre_R
      }break;
    }
  }
  {
state_label_28: // pre_u
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_2; // identifier
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_2; // identifier
      }break;
      case 0x22:
      {
        state.ptr += 1;
        goto state_label_32; // string
      }break;
      case 0x38:
      {
        state.ptr += 1;
        state.flags_ZF0 |= 0x8;
        goto state_label_30; // pre_u8
      }break;
      case 0x52:
      {
        state.ptr += 1;
        goto state_label_31; // pre_R
      }break;
    }
  }
  {
state_label_29: // pre_U
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_2; // identifier
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_2; // identifier
      }break;
      case 0x22:
      {
        state.ptr += 1;
        goto state_label_32; // string
      }break;
      case 0x52:
      {
        state.ptr += 1;
        goto state_label_31; // pre_R
      }break;
    }
  }
  {
state_label_30: // pre_u8
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_2; // identifier
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_2; // identifier
      }break;
      case 0x22:
      {
        state.ptr += 1;
        goto state_label_32; // string
      }break;
      case 0x52:
      {
        state.ptr += 1;
        goto state_label_31; // pre_R
      }break;
    }
  }
  {
state_label_31: // pre_R
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_2; // identifier
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_2; // identifier
      }break;
      case 0x22:
      {
        state.ptr += 1;
        state.delim_first = state.ptr;
        goto state_label_45; // raw_string_get_delim
      }break;
    }
  }
  {
state_label_32: // string
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        state.ptr += 1;
        goto state_label_32; // string
      }break;
      case 0x0a:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x22:
      {
        if ((!HasFlag(state.flags_ZF0, 0x40))){
          state.ptr += 1;
          {
            Token token = {};
            token.pos = (i64)(state.emit_ptr - state.base);
            token.size = (i64)(state.ptr - state.emit_ptr);
            token.flags = state.flags_ZB0;
            do{
              if (HasFlag(state.flags_ZF0, 0x4)){
                token.sub_kind = TokenJaiKind_LiteralStringWide;
                token.kind = 10;
                break;
              }
              if (HasFlag(state.flags_ZF0, 0x8)){
                token.sub_kind = TokenJaiKind_LiteralStringUTF8;
                token.kind = 10;
                break;
              }
              if (HasFlag(state.flags_ZF0, 0x10)){
                token.sub_kind = TokenJaiKind_LiteralStringUTF16;
                token.kind = 10;
                break;
              }
              if (HasFlag(state.flags_ZF0, 0x20)){
                token.sub_kind = TokenJaiKind_LiteralStringUTF32;
                token.kind = 10;
                break;
              }
              token.sub_kind = TokenJaiKind_LiteralString;
              token.kind = 10;
            }while(0);
            token_list_push(arena, list, &token);
            emit_counter += 1;
            state.emit_ptr = state.ptr;
            if (emit_counter == max){
              goto end;
            }
          }
          state.flags_ZF0 = 0;
          state.flags_ZB0 = 0;
          goto state_label_1; // root
        }
        state.ptr += 1;
        goto state_label_32; // string
      }break;
      case 0x27:
      {
        if ((HasFlag(state.flags_ZF0, 0x40))){
          state.ptr += 1;
          {
            Token token = {};
            token.pos = (i64)(state.emit_ptr - state.base);
            token.size = (i64)(state.ptr - state.emit_ptr);
            token.flags = state.flags_ZB0;
            do{
              if (HasFlag(state.flags_ZF0, 0x4)){
                token.sub_kind = TokenJaiKind_LiteralCharacterWide;
                token.kind = 10;
                break;
              }
              if (HasFlag(state.flags_ZF0, 0x8)){
                token.sub_kind = TokenJaiKind_LiteralCharacterUTF8;
                token.kind = 10;
                break;
              }
              if (HasFlag(state.flags_ZF0, 0x10)){
                token.sub_kind = TokenJaiKind_LiteralCharacterUTF16;
                token.kind = 10;
                break;
              }
              if (HasFlag(state.flags_ZF0, 0x20)){
                token.sub_kind = TokenJaiKind_LiteralCharacterUTF32;
                token.kind = 10;
                break;
              }
              token.sub_kind = TokenJaiKind_LiteralCharacter;
              token.kind = 10;
            }while(0);
            token_list_push(arena, list, &token);
            emit_counter += 1;
            state.emit_ptr = state.ptr;
            if (emit_counter == max){
              goto end;
            }
          }
          state.flags_ZF0 = 0;
          state.flags_ZB0 = 0;
          goto state_label_1; // root
        }
        state.ptr += 1;
        goto state_label_32; // string
      }break;
      case 0x5c:
      {
        state.ptr += 1;
        goto state_label_33; // string_esc
      }break;
    }
  }
  {
state_label_33: // string_esc
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_EOF;
            token.kind = 0;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        result = true;
        goto end;
      }
    }
    switch (*state.ptr){
      default:
      {
        state.ptr += 1;
        goto state_label_32; // string
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:
      {
        state.ptr += 1;
        goto state_label_34; // string_esc_oct2
      }break;
      case 0x55:
      {
        state.ptr += 1;
        goto state_label_37; // string_esc_universal_8
      }break;
      case 0x75:
      {
        state.ptr += 1;
        goto state_label_41; // string_esc_universal_4
      }break;
      case 0x78:
      {
        state.ptr += 1;
        goto state_label_36; // string_esc_hex
      }break;
    }
  }
  {
state_label_34: // string_esc_oct2
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_32; // string
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_32; // string
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:
      {
        state.ptr += 1;
        goto state_label_35; // string_esc_oct1
      }break;
    }
  }
  {
state_label_35: // string_esc_oct1
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_32; // string
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_32; // string
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:
      {
        state.ptr += 1;
        goto state_label_32; // string
      }break;
    }
  }
  {
state_label_36: // string_esc_hex
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_32; // string
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_32; // string
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
      case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
      case 0x66:
      {
        state.ptr += 1;
        goto state_label_36; // string_esc_hex
      }break;
    }
  }
  {
state_label_37: // string_esc_universal_8
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_32; // string
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_32; // string
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
      case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
      case 0x66:
      {
        state.ptr += 1;
        goto state_label_38; // string_esc_universal_7
      }break;
    }
  }
  {
state_label_38: // string_esc_universal_7
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_32; // string
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_32; // string
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
      case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
      case 0x66:
      {
        state.ptr += 1;
        goto state_label_39; // string_esc_universal_6
      }break;
    }
  }
  {
state_label_39: // string_esc_universal_6
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_32; // string
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_32; // string
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
      case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
      case 0x66:
      {
        state.ptr += 1;
        goto state_label_40; // string_esc_universal_5
      }break;
    }
  }
  {
state_label_40: // string_esc_universal_5
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_32; // string
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_32; // string
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
      case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
      case 0x66:
      {
        state.ptr += 1;
        goto state_label_41; // string_esc_universal_4
      }break;
    }
  }
  {
state_label_41: // string_esc_universal_4
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_32; // string
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_32; // string
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
      case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
      case 0x66:
      {
        state.ptr += 1;
        goto state_label_42; // string_esc_universal_3
      }break;
    }
  }
  {
state_label_42: // string_esc_universal_3
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_32; // string
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_32; // string
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
      case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
      case 0x66:
      {
        state.ptr += 1;
        goto state_label_43; // string_esc_universal_2
      }break;
    }
  }
  {
state_label_43: // string_esc_universal_2
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_32; // string
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_32; // string
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
      case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
      case 0x66:
      {
        state.ptr += 1;
        goto state_label_44; // string_esc_universal_1
      }break;
    }
  }
  {
state_label_44: // string_esc_universal_1
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_32; // string
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_32; // string
      }break;
      case 0x30:case 0x31:case 0x32:case 0x33:case 0x34:case 0x35:case 0x36:
      case 0x37:case 0x38:case 0x39:case 0x41:case 0x42:case 0x43:case 0x44:
      case 0x45:case 0x46:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:
      case 0x66:
      {
        state.ptr += 1;
        goto state_label_32; // string
      }break;
    }
  }
  {
state_label_45: // raw_string_get_delim
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_EOF;
            token.kind = 0;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        result = true;
        goto end;
      }
    }
    switch (*state.ptr){
      default:
      {
        state.ptr += 1;
        goto state_label_45; // raw_string_get_delim
      }break;
      case 0x20:case 0x29:case 0x5c:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x28:
      {
        state.delim_one_past_last = state.ptr;
        state.ptr += 1;
        goto state_label_46; // raw_string_find_close
      }break;
    }
  }
  {
state_label_46: // raw_string_find_close
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LexError;
            token.kind = 2;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_EOF;
            token.kind = 0;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        result = true;
        goto end;
      }
    }
    switch (*state.ptr){
      default:
      {
        state.ptr += 1;
        goto state_label_46; // raw_string_find_close
      }break;
      case 0x29:
      {
        state.ptr += 1;
        goto state_label_47; // raw_string_try_delim
      }break;
    }
  }
  {
state_label_47: // raw_string_try_delim
    u64 delim_length = state.delim_one_past_last - state.delim_first;
    u64 parse_length = 0;
    for (;;){
      if (parse_length == delim_length){
        goto state_label_48; // raw_string_try_quote
      }
      if (state.ptr == state.opl_ptr){
        goto state_label_48; // raw_string_try_quote
      }
      if (*state.ptr == state.delim_first[parse_length]){
        state.ptr += 1;
        parse_length += 1;
      }
      else{
        goto state_label_46; // raw_string_find_close
      }
    }
  }
  {
state_label_48: // raw_string_try_quote
    if (state.ptr == state.opl_ptr){
      if ((true)){
        goto state_label_46; // raw_string_find_close
      }
    }
    switch (*state.ptr){
      default:
      {
        goto state_label_46; // raw_string_find_close
      }break;
      case 0x22:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            if (HasFlag(state.flags_ZF0, 0x4)){
              token.sub_kind = TokenJaiKind_LiteralStringWideRaw;
              token.kind = 10;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x8)){
              token.sub_kind = TokenJaiKind_LiteralStringUTF8Raw;
              token.kind = 10;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x10)){
              token.sub_kind = TokenJaiKind_LiteralStringUTF16Raw;
              token.kind = 10;
              break;
            }
            if (HasFlag(state.flags_ZF0, 0x20)){
              token.sub_kind = TokenJaiKind_LiteralStringUTF32Raw;
              token.kind = 10;
              break;
            }
            token.sub_kind = TokenJaiKind_LiteralStringRaw;
            token.kind = 10;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_49: // comment_block
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_BlockComment;
            token.kind = 3;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_EOF;
            token.kind = 0;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        result = true;
        goto end;
      }
    }
    switch (*state.ptr){
      default:
      {
        state.ptr += 1;
        goto state_label_49; // comment_block
      }break;
      case 0x0a:
      {
        state.ptr += 1;
        state.flags_ZB0 &= ~(0x1);
        state.flags_KF0 &= ~(0x1);
        goto state_label_49; // comment_block
      }break;
      case 0x2a:
      {
        state.ptr += 1;
        goto state_label_50; // comment_block_try_close
      }break;
    }
  }
  {
state_label_50: // comment_block_try_close
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_BlockComment;
            token.kind = 3;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_EOF;
            token.kind = 0;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        result = true;
        goto end;
      }
    }
    switch (*state.ptr){
      default:
      {
        state.ptr += 1;
        goto state_label_49; // comment_block
      }break;
      case 0x2a:
      {
        state.ptr += 1;
        goto state_label_50; // comment_block_try_close
      }break;
      case 0x2f:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_BlockComment;
            token.kind = 3;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_51: // comment_line
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LineComment;
            token.kind = 3;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        state.ptr += 1;
        goto state_label_51; // comment_line
      }break;
      case 0x0a:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LineComment;
            token.kind = 3;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x5c:
      {
        state.ptr += 1;
        goto state_label_52; // comment_line_backslashing
      }break;
    }
  }
  {
state_label_52: // comment_line_backslashing
    if (state.ptr == state.opl_ptr){
      if ((true)){
        result = true;
        goto end;
      }
    }
    switch (*state.ptr){
      default:
      {
        state.ptr += 1;
        goto state_label_51; // comment_line
      }break;
      case 0x0d:
      {
        state.ptr += 1;
        goto state_label_52; // comment_line_backslashing
      }break;
    }
  }
  {
state_label_53: // op stage
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Colon;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Colon;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3a:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_ColonColon;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3d:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_ColonEq;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_54: // op stage
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Minus;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Minus;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3d:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_MinusEq;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3e:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_MinusGrtr;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_55: // op stage
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Eq;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Eq;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3d:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_EqEq;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3e:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_EqGrtr;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_56: // op stage
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Less;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Less;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3c:
      {
        state.ptr += 1;
        goto state_label_64; // op stage
      }break;
      case 0x3d:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LessEq;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_57: // op stage
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Grtr;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Grtr;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3d:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_GrtrEq;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3e:
      {
        state.ptr += 1;
        goto state_label_65; // op stage
      }break;
    }
  }
  {
state_label_58: // op stage
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Not;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Not;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3d:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_NotEq;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_59: // op stage
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_And;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_And;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x26:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_AndAnd;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_60: // op stage
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Or;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Or;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x7c:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_OrOr;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_61: // op stage
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Plus;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Plus;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3d:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_PlusEq;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_62: // op stage
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Star;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Star;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3d:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_StarEq;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_63: // op stage
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Mod;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_Mod;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3d:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_ModEq;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_64: // op stage
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LeftLeft;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LeftLeft;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3d:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_LeftLeftEq;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_65: // op stage
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_RightRight;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_RightRight;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x3d:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_RightRightEq;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
  {
state_label_66: // op stage
    if (state.ptr == state.opl_ptr){
      if ((true)){
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_DotDot;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }
    }
    switch (*state.ptr){
      default:
      {
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_DotDot;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
      case 0x2e:
      {
        state.ptr += 1;
        {
          Token token = {};
          token.pos = (i64)(state.emit_ptr - state.base);
          token.size = (i64)(state.ptr - state.emit_ptr);
          token.flags = state.flags_ZB0;
          do{
            token.sub_kind = TokenJaiKind_DotDotDot;
            token.kind = 7;
          }while(0);
          token_list_push(arena, list, &token);
          emit_counter += 1;
          state.emit_ptr = state.ptr;
          if (emit_counter == max){
            goto end;
          }
        }
        state.flags_ZF0 = 0;
        state.flags_ZB0 = 0;
        goto state_label_1; // root
      }break;
    }
  }
end:;
  block_copy_struct(state_ptr, &state);
  return(result);
}
internal Token_List
lex_full_input_jai(Arena *arena, String_Const_u8 input){
  Lex_State_Jai state = {};
  lex_full_input_jai_init(&state, input);
  Token_List list = {};
  lex_full_input_jai_breaks(arena, &list, &state, max_u64);
  return(list);
}
