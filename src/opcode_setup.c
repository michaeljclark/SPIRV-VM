#include <spvm/context.h>
#include <spvm/opcode.h>
#include <spvm/state.h>
#include <spvm/spirv.h>

/* 3.32.2 Debug Instructions */
void spvm_setup_OpSource(spvm_word word_count, spvm_state_t state)
{
	state->owner->language = SPVM_READ_WORD(state->code_current);
	state->owner->language_version = SPVM_READ_WORD(state->code_current);
}
void spvm_setup_OpSourceExtension(spvm_word word_count, spvm_state_t state)
{
	spvm_string ext = spvm_program_add_extension(state->owner, word_count);
	spvm_string_read(state->code_current, ext, word_count);
}
void spvm_setup_OpName(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	state->results[id].name = (spvm_string)malloc(word_count * sizeof(spvm_word) + 1);
	spvm_string_read(state->code_current, state->results[id].name, word_count - 1);
}
void spvm_setup_OpMemberName(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word memb = SPVM_READ_WORD(state->code_current);

	state->results[id].member_name_count = MAX(memb + 1, state->results[id].member_name_count);
	state->results[id].member_name = (spvm_string*)realloc(state->results[id].member_name, sizeof(spvm_string) * state->results[id].member_name_count);

	spvm_word slen = word_count - 2;
	state->results[id].member_name[memb] = (spvm_string)malloc(sizeof(spvm_word) * slen);
	spvm_string_read(state->code_current, state->results[id].member_name[memb], slen);
}
void spvm_setup_OpString(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	state->results[id].type = spvm_result_type_string;
	state->results[id].name = (spvm_string)malloc((word_count - 1) * sizeof(spvm_word) + 1);
	spvm_string_read(state->code_current, state->results[id].name, word_count - 1);
}

/* 3.32.3 Annotation Instructions */
void spvm_setup_OpDecorate(spvm_word word_count, spvm_state_t state)
{
	spvm_word target = SPVM_READ_WORD(state->code_current);
	SpvDecoration decor = SPVM_READ_WORD(state->code_current);
	spvm_word literal1 = 0, literal2 = 0;

	spvm_decoration_read(state->code_current, decor, &literal1, &literal2);
	spvm_result_add_decoration(&state->results[target], decor, literal1, literal2);
}
void spvm_setup_OpMemberDecorate(spvm_word word_count, spvm_state_t state)
{
	spvm_word target = SPVM_READ_WORD(state->code_current);
	SpvDecoration memb = SPVM_READ_WORD(state->code_current);
	SpvDecoration decor = SPVM_READ_WORD(state->code_current);
	spvm_word literal1 = 0, literal2 = 0;

	spvm_decoration_read(state->code_current, decor, &literal1, &literal2);
	spvm_result_add_member_decoration(&state->results[target], decor, literal1, literal2, memb);
}

/* 3.32.4 Extension Instructions */
void spvm_setup_OpExtInstImport(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word name_index = state->owner->import_count;

	word_count--;

	spvm_string imp = (spvm_string)calloc(word_count, sizeof(spvm_word));
	spvm_string_read(state->code_current, imp, word_count);

	state->results[id].type = spvm_result_type_extension;
	state->results[id].name = imp;
}

/* 3.32.5 Mode-Setting Instructions */
void spvm_setup_OpMemoryModel(spvm_word word_count, spvm_state_t state)
{
	state->owner->addressing = SPVM_READ_WORD(state->code_current);
	state->owner->memory_model = SPVM_READ_WORD(state->code_current);
}
void spvm_setup_OpEntryPoint(spvm_word word_count, spvm_state_t state)
{
	spvm_entry_point* entry = spvm_program_create_entry_point(state->owner);
	entry->exec_model = SPVM_READ_WORD(state->code_current);
	entry->id = SPVM_READ_WORD(state->code_current);

	spvm_word name_length = 0;
	entry->name = spvm_string_read_all(state->code_current, &name_length);
	state->code_current += name_length;

	spvm_word interface_count = word_count - name_length - 2;
	entry->globals_count = interface_count;
	
	if (interface_count) {
		entry->globals = (spvm_word*)calloc(interface_count, sizeof(spvm_word));
		spvm_word interface_index = 0;
		while (interface_count) {
			entry->globals[interface_index] = SPVM_READ_WORD(state->code_current);
			interface_index++;
			interface_count--;
		}
	}
}
void spvm_setup_OpCapability(spvm_word word_count, spvm_state_t state)
{
	SpvCapability cap = SPVM_READ_WORD(state->code_current);
	spvm_program_add_capability(state->owner, cap);
}

/* 3.32.6 Type-Declaration Instructions */
void spvm_setup_OpTypeVoid(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_void;
	state->results[store_id].value_bitmask = 0;
	state->results[store_id].member_count = 0;
}
void spvm_setup_OpTypeBool(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_bool;
	state->results[store_id].value_bitmask = 1;
	state->results[store_id].member_count = 1;
}
void spvm_setup_OpTypeInt(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_int;
	state->results[store_id].value_bitmask = SPVM_READ_WORD(state->code_current);
	state->results[store_id].value_sign = SPVM_READ_WORD(state->code_current);
	state->results[store_id].member_count = 1;
}
void spvm_setup_OpTypeFloat(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_float;
	state->results[store_id].value_bitmask = SPVM_READ_WORD(state->code_current);
	state->results[store_id].member_count = 1;
}
void spvm_setup_OpTypeVector(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_vector;

	state->results[store_id].pointer = SPVM_READ_WORD(state->code_current);
	state->results[store_id].member_count = SPVM_READ_WORD(state->code_current);
}
void spvm_setup_OpTypeMatrix(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_matrix;

	state->results[store_id].pointer = SPVM_READ_WORD(state->code_current);
	state->results[store_id].member_count = SPVM_READ_WORD(state->code_current);
}
void spvm_setup_OpTypeImage(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	state->results[id].type = spvm_result_type_type;
	state->results[id].value_type = spvm_value_type_image;
	state->results[id].pointer = SPVM_READ_WORD(state->code_current);
	state->results[id].image_dimension = SPVM_READ_WORD(state->code_current);
	state->results[id].member_count = 1;
}
void spvm_setup_OpTypeSampledImage(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	state->results[id].type = spvm_result_type_type;
	state->results[id].value_type = spvm_value_type_sampled_image;
	state->results[id].pointer = SPVM_READ_WORD(state->code_current);
	state->results[id].member_count = 1;
}
void spvm_setup_OpTypeArray(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_array;

	state->results[store_id].pointer = SPVM_READ_WORD(state->code_current);
	state->results[store_id].member_count = SPVM_READ_WORD(state->code_current);
}
void spvm_setup_OpTypeStruct(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word mcnt = word_count - 1;
	state->results[id].type = spvm_result_type_type;
	state->results[id].value_type = spvm_value_type_struct;
	state->results[id].member_count = mcnt;

	state->results[id].params = (spvm_word*)malloc(sizeof(spvm_word) * mcnt);
	for (spvm_word i = 0; i < mcnt; i++)
		state->results[id].params[i] = SPVM_READ_WORD(state->code_current);
}
void spvm_setup_OpTypePointer(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);

	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_pointer;
	state->results[store_id].storage_class = SPVM_READ_WORD(state->code_current);
	state->results[store_id].pointer = SPVM_READ_WORD(state->code_current);
}
void spvm_setup_OpTypeFunction(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word return_id = SPVM_READ_WORD(state->code_current);
	spvm_word param_count = word_count - 2;

	state->results[id].type = spvm_result_type_function_type;
	state->results[id].pointer = return_id;
	state->results[id].member_count = param_count;
	state->results[id].params = (spvm_word*)malloc(param_count * sizeof(spvm_word));

	for (spvm_word i = 0; i < param_count; i++)
		state->results[id].params[i] = SPVM_READ_WORD(state->code_current);
}

/* 3.32.7 Constant-Creation Instructions */
void spvm_setup_OpConstantTrue(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = 1;
}
void spvm_setup_OpConstantFalse(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = 0;
}
void spvm_setup_OpConstantNull(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);
}
void spvm_setup_OpConstant(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);

	state->results[id].members[0].value.u64 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[var_type]);
	if (type_info->value_bitmask > 32) {
		unsigned long long highBits = SPVM_READ_WORD(state->code_current);
		state->results[id].members[0].value.u64 |= highBits << 32ull;
	}
}
void spvm_setup_OpConstantComposite(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		spvm_word index = SPVM_READ_WORD(state->code_current);
		state->results[id].members[i].value.s = state->results[index].members[0].value.s;
	}
}

/* 3.32.8 Memory Instructions */
void spvm_setup_OpVariable(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_variable;
	state->results[id].storage_class = SPVM_READ_WORD(state->code_current);
	state->results[id].owner = state->current_function;

	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);
}
void spvm_setup_OpLoad(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word ptr_id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);
}
void spvm_setup_OpFunction(spvm_word word_count, spvm_state_t state)
{
	spvm_word ret_type = SPVM_READ_WORD(state->code_current);
	spvm_word store_id = SPVM_READ_WORD(state->code_current);

	state->results[store_id].type = spvm_result_type_function;
	state->results[store_id].return_type = ret_type;

	SPVM_SKIP_WORD(state->code_current); // skip function control

	spvm_word info = state->results[store_id].pointer = SPVM_READ_WORD(state->code_current);

	state->results[store_id].source_location = state->code_current;
	state->results[info].source_location = state->code_current;

	state->current_parameter = 0;
	state->results[store_id].member_count = state->results[info].member_count;
	state->results[store_id].params = (spvm_word*)calloc(state->results[store_id].member_count, sizeof(spvm_word));

	state->current_function = &state->results[store_id];
}
void spvm_setup_OpFunctionParameter(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_function_parameter;
	state->results[id].storage_class = SPVM_READ_WORD(state->code_current);
	state->current_function->params[state->current_parameter] = id;
	state->current_parameter++;

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[var_type]);

	state->results[id].pointer = var_type;
	state->results[id].member_count = type_info->member_count;
	state->results[id].owner = state->current_function;
}
void spvm_setup_OpFunctionEnd(spvm_word word_count, spvm_state_t state)
{
	state->current_function = NULL;
}
void spvm_setup_OpFunctionCall(spvm_word word_count, spvm_state_t state)
{
	spvm_word ret_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, ret_type);
}

/* 3.32.11 Conversion Instructions */
/* 3.32.13 Arithmetic Instructions */
/* 3.32.14 Bit Instructions */
void spvm_setup_constant(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);
}

/* 3.32.17 Control-Flow Instructions */
void spvm_setup_OpLabel(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_label;
	state->results[id].source_location = state->code_current;
}

void _spvm_context_create_setup_table(spvm_context_t ctx)
{
	ctx->opcode_setup = (spvm_opcode_func*)calloc(SPVM_OPCODE_TABLE_LENGTH, sizeof(spvm_opcode_func));

	ctx->opcode_setup[SpvOpSource] = spvm_setup_OpSource;
	ctx->opcode_setup[SpvOpSourceExtension] = spvm_setup_OpSourceExtension;
	ctx->opcode_setup[SpvOpName] = spvm_setup_OpName;
	ctx->opcode_setup[SpvOpMemberName] = spvm_setup_OpMemberName;
	ctx->opcode_setup[SpvOpString] = spvm_setup_OpString;

	ctx->opcode_setup[SpvOpDecorate] = spvm_setup_OpDecorate;
	ctx->opcode_setup[SpvOpMemberDecorate] = spvm_setup_OpMemberDecorate;

	ctx->opcode_setup[SpvOpExtInstImport] = spvm_setup_OpExtInstImport;
	ctx->opcode_setup[SpvOpExtInst] = spvm_setup_constant;

	ctx->opcode_setup[SpvOpMemoryModel] = spvm_setup_OpMemoryModel;
	ctx->opcode_setup[SpvOpEntryPoint] = spvm_setup_OpEntryPoint;
	ctx->opcode_setup[SpvOpCapability] = spvm_setup_OpCapability;

	ctx->opcode_setup[SpvOpTypeVoid] = spvm_setup_OpTypeVoid;
	ctx->opcode_setup[SpvOpTypeBool] = spvm_setup_OpTypeBool;
	ctx->opcode_setup[SpvOpTypeInt] = spvm_setup_OpTypeInt;
	ctx->opcode_setup[SpvOpTypeFloat] = spvm_setup_OpTypeFloat;
	ctx->opcode_setup[SpvOpTypeVector] = spvm_setup_OpTypeVector;
	ctx->opcode_setup[SpvOpTypeMatrix] = spvm_setup_OpTypeMatrix;
	ctx->opcode_setup[SpvOpTypeImage] = spvm_setup_OpTypeImage;
	ctx->opcode_setup[SpvOpTypeSampledImage] = spvm_setup_OpTypeSampledImage;
	ctx->opcode_setup[SpvOpTypeArray] = spvm_setup_OpTypeArray;
	ctx->opcode_setup[SpvOpTypeStruct] = spvm_setup_OpTypeStruct;
	ctx->opcode_setup[SpvOpTypePointer] = spvm_setup_OpTypePointer;
	ctx->opcode_setup[SpvOpTypeFunction] = spvm_setup_OpTypeFunction;

	ctx->opcode_setup[SpvOpConstantTrue] = spvm_setup_OpConstantTrue;
	ctx->opcode_setup[SpvOpConstantFalse] = spvm_setup_OpConstantFalse;
	ctx->opcode_setup[SpvOpConstantNull] = spvm_setup_OpConstantNull;
	ctx->opcode_setup[SpvOpConstant] = spvm_setup_OpConstant;
	ctx->opcode_setup[SpvOpConstantComposite] = spvm_setup_OpConstantComposite;

	ctx->opcode_setup[SpvOpVariable] = spvm_setup_OpVariable;
	ctx->opcode_setup[SpvOpLoad] = spvm_setup_OpLoad;
	ctx->opcode_setup[SpvOpFunction] = spvm_setup_OpFunction;
	ctx->opcode_setup[SpvOpFunctionParameter] = spvm_setup_OpFunctionParameter;
	ctx->opcode_setup[SpvOpFunctionEnd] = spvm_setup_OpFunctionEnd;
	ctx->opcode_setup[SpvOpFunctionCall] = spvm_setup_OpFunctionCall;
	ctx->opcode_setup[SpvOpPtrEqual] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpPtrNotEqual] = spvm_setup_constant;

	ctx->opcode_setup[SpvOpConvertFToU] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpConvertFToS] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpConvertUToF] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpConvertSToF] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpFConvert] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpBitcast] = spvm_setup_constant;

	ctx->opcode_setup[SpvOpVectorExtractDynamic] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpVectorInsertDynamic] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpVectorShuffle] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpCompositeConstruct] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpCompositeExtract] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpCopyObject] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpTranspose] = spvm_setup_constant;

	ctx->opcode_setup[SpvOpSNegate] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpFNegate] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpIAdd] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpFAdd] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpISub] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpFSub] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpIMul] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpFMul] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpUDiv] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpSDiv] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpFDiv] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpUMod] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpSMod] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpFMod] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpVectorTimesScalar] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpMatrixTimesScalar] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpVectorTimesMatrix] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpMatrixTimesVector] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpMatrixTimesMatrix] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpOuterProduct] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpDot] = spvm_setup_constant;

	ctx->opcode_setup[SpvOpBitwiseOr] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpBitwiseAnd] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpBitwiseXor] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpNot] = spvm_setup_constant;

	ctx->opcode_setup[SpvOpAny] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpAll] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpIsNan] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpIsInf] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpLogicalEqual] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpLogicalNotEqual] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpLogicalAnd] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpLogicalOr] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpLogicalNot] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpSelect] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpIEqual] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpINotEqual] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpUGreaterThan] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpSGreaterThan] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpUGreaterThanEqual] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpSGreaterThanEqual] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpULessThan] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpSLessThan] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpULessThanEqual] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpSLessThanEqual] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpFOrdEqual] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpFOrdNotEqual] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpFOrdLessThan] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpFOrdGreaterThan] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpFOrdLessThanEqual] = spvm_setup_constant;
	ctx->opcode_setup[SpvOpFOrdGreaterThanEqual] = spvm_setup_constant;

	ctx->opcode_setup[SpvOpImageSampleImplicitLod] = spvm_setup_constant;

	ctx->opcode_setup[SpvOpLabel] = spvm_setup_OpLabel;
}