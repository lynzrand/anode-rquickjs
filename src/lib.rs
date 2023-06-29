//! # High-level bindings to quickjs
//!
//! The `rquickjs` crate provides safe high-level bindings to the
//! [quickjs](https://bellard.org/quickjs/) javascript engine. This crate is heavily inspired by
//! the [rlua](https://crates.io/crates/rlua) crate.
//!
//! # The `Runtime` and `Context` objects
//!
//! The main entry point of this library is the [`Runtime`] struct. It represents the interperter
//! state and is used to create [`Context`] objects. As the quickjs library does not support
//! threading the runtime is locked behind a mutex. Multiple threads cannot run as script or create
//! objects from the same runtime at the same time. The [`Context`] object represents a global
//! environment. Contexts of the same runtime can share javascript objects like in browser between
//! frames of the same origin.
//!
//! # Converting Values
//!
//! This library has multiple traits for converting to and from javascript. The [`IntoJs`] trait is
//! used for taking rust values and turning them into javascript values. The [`FromJs`] is for
//! converting javascript value to rust. Note that this trait does not automatic coercion but the
//! [`Coerced`](convert::Coerced) can be used to convert the values with coercion.
//!
//! For values which represent the name of variables or indecies the trait [`IntoAtom`] is
//! available to convert values to the represention quickjs requires.
//!
//! # Optional features
//!
//! ## Default
//!
//! This crate can be customized via features. The following features is enabled by default but can
//! be disabled when not needed: - `classes` enables support for ES6 classes. Any user-defined Rust
//! type can be exported to JS as an ES6 class which can be derived and extended by JS. -
//! `properties` enables support for object properties (`Object.defineProperty`). - `exports` adds
//! an ability to read the module exports.
//!
//! ## Advanced
//!
//! The following features may be enabled to get an extra functionality:
//!
//! - `allocator` adds support for custom allocators for [`Runtime`]. The allocators should
//! implements [`std::alloc::Allocator`] trait and can be plugged on [`Runtime`] creation via
//! [`Runtime::new_with_alloc`].
//!
//! - `rust-alloc` forces using Rust's global allocator by default instead of libc's one.
//!
//! - `loader` adds support for custom ES6 modules resolvers and loaders. The resolvers and loaders
//! should implements [`Resolver`](loader::Resolver) and [Loader](loader::Loader) traits
//! respectively and can be plugged in already existing [`Runtime`] before loading modules via
//! [`Runtime::set_loader`]. The resolvers and loaders can be easily combined via tuples. When the
//! previous resolver or loader failed the next one will be applied.
//!
//! - `dyn-load` adds support for loadable native modules (so/dll/dylib).
//!
//! - `array-buffer` adds support for [`ArrayBuffer`] and [`TypedArray`].
//!
//! - `futures` adds support for async Rust. When enabled the library exports [`AsyncRuntime`] and
//! [`AsyncContext`]. These are the asynchronous variants of the normal runtime and context. In
//! order to ensure that QuickJS is used properly the runtime is placed behind a lock. For the
//! normal runtime this is a normal mutex. You should avoid blocking threads in asynchronous rust
//! so the async runtime uses a future aware mutex. In the async runtime Rust futures can be passed
//! to JS as [ES6
//! Promises](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise)
//! and ES6 Promises can be given back as Rust futures.
//!
//! - `macro` enables some useful procedural macros which gets Rust/JS interop much easy. An
//! [attribute](#attributes) macros can be applied to functions, constants and modules. An
//! [derive](#derives) macros can be used with structs and enums.
//!
//! - `phf` enables using Perfect Hash Function for builtin modules lookup
//!
//! ## Custom
//!
//! To gets build faster the numbers of arguments which can be passed to and given by the functions
//! is limited to 6. If you need more arguments you can enabled feature `max-args-N` where N is
//! number from 7 to 16.
//!
//! ## Extra types
//!
//! This crate has support for conversion of many Rust types like [`Option`](std::option::Option),
//! [`Result`](std::result::Result), [`Vec`] and other collections. In addition an extra types
//! support can be enabled via features:
//!
//! - `either` adds [`FromJs`]/[`IntoJs`] implementations for [`Either`](`either::Either`)
//!
//! - `indexmap` adds [`FromJs`]/[`IntoJs`] implementations for [`IndexSet`](`indexmap::IndexSet`)
//! and [`IndexMap`](`indexmap_rs::IndexMap`)
//!
//! ## Bindings
//!
//! The bindings is pre-built for the following platforms:
//!
//! - `i686-unknown-linux-gnu`, `x86_64-unknown-linux-gnu`
//!
//! - `x86_64-apple-darwin`
//!
//! - `i686-pc-windows-gnu`, `x86_64-pc-windows-gnu`, `i686-pc-windows-msvc`,
//! `x86_64-pc-windows-msvc`
//!
//! To build the crate for unsupported target you must enable `bindgen` feature.
//!
//! ## Experimental
//!
//! - `parallel` enables multithreading support.
//!
//! Note that the experimental features which may not works as expected. Use it for your own risk.
//!
//! ## Debugging
//!
//! QuickJS can be configured to output some info which can help debug. The following features
//! enables that:
//! - `dump-bytecode`
//! - `dump-gc`
//! - `dump-gc-free`
//! - `dump-free`
//! - `dump-leaks`
//! - `dump-mem`
//! - `dump-objects`
//! - `dump-atoms`
//! - `dump-shapes`
//! - `dump-module-resolve`
//! - `dump-promise`
//! - `dump-read-object`

#![cfg_attr(feature = "doc-cfg", feature(doc_cfg))]

pub use rquickjs_core::*;

#[cfg_attr(feature = "doc-cfg", doc(cfg(feature = "macro")))]
#[cfg(feature = "macro")]
pub use rquickjs_macro::{bind, embed, FromJs, HasRefs, IntoJs, IntoJsByRef};

// The following imports needed to linking docs

#[cfg(feature = "either")]
#[cfg_attr(feature = "doc-cfg", doc(cfg(feature = "either")))]
extern crate either_rs as either;

#[cfg(feature = "indexmap")]
#[cfg_attr(feature = "doc-cfg", doc(cfg(feature = "indexmap")))]
extern crate indexmap_rs as indexmap;
