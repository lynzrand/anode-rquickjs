use crate::{qjs, Ctx, Error, Result, String, Value};
use std::{ffi::CStr, string::String as StdString};

/// An atom is value representing the name of a variable of an objects and can be created
/// from any javascript value.
///
/// # Representation
///
/// Atoms in quickjs are handled differently depending on what type of index the represent.
/// When the atom represents a number like index, like `object[1]` the atom is just
/// a normal number.
/// However when the atom represents a string link index like `object["foo"]` or `object.foo`
/// the atom represents a value in a hashmap.
pub struct Atom<'js> {
    pub(crate) atom: qjs::JSAtom,
    ctx: Ctx<'js>,
}

impl<'js> Atom<'js> {
    /// Create a atom from a javascript value.
    pub fn from_value(ctx: Ctx<'js>, val: &Value<'js>) -> Result<Atom<'js>> {
        let atom = unsafe { qjs::JS_ValueToAtom(ctx.as_ptr(), val.as_js_value()) };
        if atom == qjs::JS_ATOM_NULL {
            // A value can be anything, including an object which might contain a callback so check
            // for panics.
            return Err(ctx.raise_exception());
        }
        Ok(Atom { atom, ctx })
    }

    /// Create a atom from a u32
    pub fn from_u32(ctx: Ctx<'js>, val: u32) -> Result<Atom<'js>> {
        let atom = unsafe { qjs::JS_NewAtomUInt32(ctx.as_ptr(), val) };
        if atom == qjs::JS_ATOM_NULL {
            // Should never invoke a callback so no panics
            return Err(Error::Exception);
        }
        Ok(Atom { atom, ctx })
    }

    /// Create a atom from an i32 via value
    pub fn from_i32(ctx: Ctx<'js>, val: i32) -> Result<Atom<'js>> {
        let atom =
            unsafe { qjs::JS_ValueToAtom(ctx.as_ptr(), qjs::JS_MKVAL(qjs::JS_TAG_INT, val)) };
        if atom == qjs::JS_ATOM_NULL {
            // Should never invoke a callback so no panics
            return Err(Error::Exception);
        }
        Ok(Atom { atom, ctx })
    }

    /// Create a atom from a bool via value
    pub fn from_bool(ctx: Ctx<'js>, val: bool) -> Result<Atom<'js>> {
        let val = if val { qjs::JS_TRUE } else { qjs::JS_FALSE };
        let atom = unsafe { qjs::JS_ValueToAtom(ctx.as_ptr(), val) };
        if atom == qjs::JS_ATOM_NULL {
            // Should never invoke a callback so no panics
            return Err(Error::Exception);
        }
        Ok(Atom { atom, ctx })
    }

    /// Create a atom from a f64 via value
    pub fn from_f64(ctx: Ctx<'js>, val: f64) -> Result<Atom<'js>> {
        let atom = unsafe { qjs::JS_ValueToAtom(ctx.as_ptr(), qjs::JS_NewFloat64(val)) };
        if atom == qjs::JS_ATOM_NULL {
            // Should never invoke a callback so no panics
            return Err(Error::Exception);
        }
        Ok(Atom { atom, ctx })
    }

    /// Create a atom from a rust string
    pub fn from_str(ctx: Ctx<'js>, name: &str) -> Result<Atom<'js>> {
        unsafe {
            let ptr = name.as_ptr() as *const std::os::raw::c_char;
            let atom = qjs::JS_NewAtomLen(ctx.as_ptr(), ptr, name.len() as _);
            if atom == qjs::JS_ATOM_NULL {
                // Should never invoke a callback so no panics
                return Err(Error::Exception);
            }
            Ok(Atom { atom, ctx })
        }
    }

    /// Convert the atom to a javascript string.
    pub fn to_string(&self) -> Result<StdString> {
        unsafe {
            let str = qjs::JS_AtomToString(self.ctx.as_ptr(), self.atom);
            if qjs::JS_IsException(str) {
                return Err(self.ctx.raise_exception());
            }
            let mut len = 0;
            let c_str = qjs::JS_ToCStringLen2(self.ctx.as_ptr(), &mut len, str, 0);
            if c_str.is_null() {
                // Might not ever happen but I am not 100% sure
                // so just incase check it.
                qjs::JS_FreeCString(self.ctx.as_ptr(), c_str);
                qjs::JS_FreeValue(self.ctx.as_ptr(), str);
                return Err(Error::Unknown);
            }
            let bytes = std::slice::from_raw_parts(c_str as *const u8, len as usize);
            // Safety: quickjs should return valid utf8 so this should be safe.
            let res = std::str::from_utf8_unchecked(bytes).to_string();
            qjs::JS_FreeCString(self.ctx.as_ptr(), c_str);
            qjs::JS_FreeValue(self.ctx.as_ptr(), str);
            Ok(res)
        }
    }

    /// Convert the atom to a javascript string .
    pub fn to_js_string(&self) -> Result<String<'js>> {
        unsafe {
            let val = qjs::JS_AtomToString(self.ctx.as_ptr(), self.atom);
            let val = self.ctx.handle_exception(val)?;
            Ok(String::from_js_value(self.ctx, val))
        }
    }

    /// Convert the atom to a javascript value.
    pub fn to_value(&self) -> Result<Value<'js>> {
        self.to_js_string().map(|String(value)| value)
    }

    /// Create an atom from raw quickjs atom value.
    ///
    /// # Safety
    ///
    /// `val` must be a valid quickjs atom value, either got from quickjs internals, or returned from
    /// [`self.as_atom_val()`].
    pub unsafe fn from_atom_val(ctx: Ctx<'js>, val: qjs::JSAtom) -> Self {
        Atom { atom: val, ctx }
    }

    /// Get the raw quickjs atom value.
    pub fn as_atom_val(&self) -> qjs::JSAtom {
        self.atom
    }

    /// Get the [`std::String`] representation of a raw atom.
    ///
    /// # Safety
    ///
    /// `atom` must be a valid quickjs atom value, either got from quickjs internals, or returned from
    /// [`self.as_atom_val()`].
    pub unsafe fn resolve_raw(ctx: Ctx<'js>, atom: qjs::JSAtom) -> Result<StdString> {
        let atom = Atom::from_atom_val(ctx, atom);
        let res = atom.to_string()?;
        std::mem::forget(atom); // We do not want to free the atom
        Ok(res)
    }

    /// Get the raw atom value from a [`&str`].
    pub fn from_str_raw(ctx: Ctx<'js>, name: &str) -> qjs::JSAtom {
        Self::from_str(ctx, name).unwrap().atom
    }
}

impl<'js> Clone for Atom<'js> {
    fn clone(&self) -> Atom<'js> {
        let atom = unsafe { qjs::JS_DupAtom(self.ctx.as_ptr(), self.atom) };
        Atom {
            atom,
            ctx: self.ctx,
        }
    }
}

impl<'js> Drop for Atom<'js> {
    fn drop(&mut self) {
        unsafe {
            qjs::JS_FreeAtom(self.ctx.as_ptr(), self.atom);
        }
    }
}
