#[used]
pub static DUMMY: &[unsafe extern "C" fn()] = &[anode_get_function_bytecode];

extern "C" {
    fn anode_get_function_bytecode();
}
