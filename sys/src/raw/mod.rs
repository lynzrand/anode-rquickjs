use crate::JSFunctionBytecode;

impl JSFunctionBytecode {
    pub fn find_line_num(&self, pc: u32) -> u32 {
        unsafe {
            crate::js_bytecode_find_line_num(std::ptr::null_mut(), self as *const _ as *mut _, pc)
                as u32
        }
    }

    pub fn find_column_num(&self, pc: u32) -> u32 {
        unsafe {
            crate::js_bytecode_find_column_num(std::ptr::null_mut(), self as *const _ as *mut _, pc)
                as u32
        }
    }

    pub fn find_line_col(&self, pc: u32) -> (u32, u32) {
        (self.find_line_num(pc), self.find_column_num(pc))
    }
}

/*
const PC2LINE_BASE: i32 = -1;
const PC2LINE_RANGE: i32 = 5;
const PC2LINE_OP_FIRST: u32 = 1;
const PC2LINE_DIFF_PC_MAX: i32 = ((255 - PC2LINE_OP_FIRST as i32) / PC2LINE_RANGE) as i32;
const PC2COLUMN_BASE: i32 = -1;
const PC2COLUMN_RANGE: i32 = 5;
const PC2COLUMN_OP_FIRST: u32 = 1;
const PC2COLUMN_DIFF_PC_MAX: i32 = ((255 - PC2COLUMN_OP_FIRST as i32) / PC2COLUMN_RANGE) as i32;
const IC_CACHE_ITEM_CAPACITY: i32 = 8;

fn find_line_num(ctx: *mut JSContext, b: *mut JSFunctionBytecode, pc_value: u32) -> i32 {
    unsafe {
        if (*b).has_debug() == 0 || (*b).debug.pc2line_buf.is_null() {
            // function was stripped
            return -1;
        }

        let mut pc = 0;
        let mut p = (*b).debug.pc2line_buf;
        let p_end = p.add((*b).debug.pc2line_len as usize);
        let mut line_num = (*b).debug.line_num;

        while p < p_end {
            let op = *p as u32;
            p = p.add(1);

            if op == 0 {
                let mut val: u32 = 0;
                let ret = crate::js_util_get_leb128(&mut val, p, p_end);

                if ret < 0 {
                    return (*b).debug.line_num;
                }

                pc += val as i32;
                p = p.add(ret as usize);
                let ret = crate::js_util_get_sleb128(&mut val as *mut _ as *mut _, p, p_end);

                if ret < 0 {
                    return (*b).debug.line_num;
                }

                p = p.add(ret as usize);
                line_num += val as i32;
            } else {
                let op = op - PC2LINE_OP_FIRST;
                pc += op as i32 / PC2LINE_RANGE;
                line_num += (op as i32 % PC2LINE_RANGE) + PC2LINE_BASE;
            }

            if pc_value < pc as u32 {
                return line_num;
            }
        }

        line_num
    }
}
*/
