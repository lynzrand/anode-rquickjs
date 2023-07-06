//! Quickjs runtime related types.

pub(crate) mod raw;

mod base;
pub use base::{Runtime, WeakRuntime};

/// The type of the interrupt handler.
#[cfg(not(feature = "parallel"))]
pub type InterruptHandler = Box<dyn FnMut() -> bool + 'static>;
/// The type of the interrupt handler.
#[cfg(feature = "parallel")]
pub type InterruptHandler = Box<dyn FnMut() -> bool + Send + 'static>;

#[cfg(feature = "futures")]
mod r#async;
#[cfg(feature = "futures")]
pub use r#async::{AsyncRuntime, AsyncWeakRuntime};
#[cfg(feature = "futures")]
mod spawner;

#[cfg(feature = "dump-rc")]
pub fn setup_dump_rc(out_file: &str) {
    let out_file = std::ffi::CString::new(out_file).unwrap();
    unsafe { crate::qjs::JS_SetUpRefCountTracing(out_file.as_ptr()) };
}

pub use crate::qjs::JSMemoryUsage as MemoryUsage;
