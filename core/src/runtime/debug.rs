use rquickjs_sys::JS_DumpAtoms;

use crate::Runtime;

impl Runtime {
    #[cfg(feature = "dump-atoms")]
    /// Dump all atoms to stdout. This function is only intended for debug usage.
    pub fn debug_dump_atoms(&self) {
        let lock = self.inner.lock();
        let ptr = lock.rt.as_ptr();
        unsafe {
            JS_DumpAtoms(ptr);
        }
        drop(lock);
    }
}
