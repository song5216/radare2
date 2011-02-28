/* radare - LGPL - Copyright 2010-2011 - pancake<nopcode.org> */

#include <r_anal.h>

R_API RAnalValue *r_anal_value_new() {
	RAnalValue *cond = R_NEW (RAnalValue);
	memset (cond, 0, sizeof (RAnalValue));
	return cond;
}

R_API RAnalValue *r_anal_value_new_from_string(const char *str) {
	/* TODO */
	return NULL;
}

// TODO: move into .h as #define free
R_API void r_anal_value_free(RAnalValue *value) {
	free (value);
}

// mul*value+regbase+regidx+delta
R_API ut64 r_anal_value_to_ut64(RAnal *anal, RAnalValue *val) {
	ut64 num;
	if (val==NULL)
		return 0LL;
	num = val->base + (val->delta*(val->mul?val->mul:1));
	if (val->reg)
		num += r_reg_get_value (anal->reg, val->reg);
	if (val->regdelta)
		num += r_reg_get_value (anal->reg, val->regdelta);
	switch (val->memref) {
	case 1:
	case 2:
	case 4:
	case 8:
		//anal->bio ...
		eprintf ("TODO: memref for to_ut64 not supported\n");
		break;
	}
	return num;
}

R_API int r_anal_value_set_ut64(RAnal *anal, RAnalValue *val, ut64 num) {
	if (val->memref) {
		if (anal->iob.io) {
			ut8 data[8];
			ut64 addr = r_anal_value_to_ut64 (anal, val);
			r_mem_set_num (data, val->memref, num, anal->big_endian);
			anal->iob.write_at (anal->iob.io, addr, data, val->memref);
		} else eprintf ("No IO binded to r_anal\n");
	} else {
		RRegItem *item = val->reg;
		if (item)
			r_reg_set_value (anal->reg, item, num);
	}
	return R_FALSE;
}

R_API char *r_anal_value_to_string (RAnalValue *value) {
	char *out = NULL;
	if (value) {
		out = r_str_new ("");
		if (!value->base && !value->reg) {
			if (value->imm != -1LL)
				out = r_str_concatf (out, "0x%"PFMT64x, value->imm);
			else out = r_str_concat (out, "-1");
		} else {
			if (value->memref) {
				switch (value->memref) {
				case 1: out = r_str_concat (out, "(char)"); break;
				case 2: out = r_str_concat (out, "(short)"); break;
				case 4: out = r_str_concat (out, "(word)"); break;
				case 8: out = r_str_concat (out, "(dword)"); break;
				}
				out = r_str_concat (out, "[");
			}
			if (value->mul) out = r_str_concatf (out, "%d*", value->mul);
			if (value->reg) out = r_str_concatf (out, "%s", value->reg->name);
			if (value->regdelta) out = r_str_concatf (out, "+%s", value->regdelta->name);
			if (value->base!=0) out = r_str_concatf (out, "0x%"PFMT64x, value->base);
			if (value->delta>0) out = r_str_concatf (out, "+0x%"PFMT64x, value->delta);
			else if (value->delta<0) out = r_str_concatf (out, "-0x%"PFMT64x, -value->delta);
			if (value->memref) out = r_str_concat (out, "]");
		}
	}
	return out;
}
