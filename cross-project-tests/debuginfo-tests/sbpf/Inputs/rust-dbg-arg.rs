// Rust port of debuginfo-tests/dexter-tests/dbg-arg.c.
#![no_std]

use core::ptr::read_volatile;

#[repr(C)]
pub struct Tag {
    tag: u64,
    pad: u64,
}
#[repr(C)]
pub struct Mtx {
    ptr: u64,
    waiters: u64,
    mtxi: Tag,
}

#[inline(never)]
#[no_mangle]
pub fn bar(i: u64, j: u64) -> u64 {
    i.wrapping_add(j)
}

#[inline(never)]
#[no_mangle]
pub fn foobar(mutex: &Mtx) -> u64 {
    let mut r: u64 = 1;
    let mut l: u64 = 0;
    loop {
        let waiters = unsafe { read_volatile(&mutex.waiters) }; // DexLabel('inspect')
        if waiters != 0 {
            r = 2;
        }
        let j = bar(r, l);
        l = l.wrapping_add(1);
        if l >= j {
            return r.wrapping_add(j);
        }
    }
}

#[no_mangle]
pub extern "C" fn entrypoint() -> u64 {
    let m = Mtx {
        ptr: 0,
        waiters: 0,
        mtxi: Tag { tag: 17, pad: 0 },
    };
    foobar(&m)
}

// Check actual field values, not just whether a reference is retrievable.
// DexExpectWatchValue('mutex.waiters', '0', on_line=ref('inspect'))
// DexExpectWatchValue('mutex.mtxi.tag', '17', on_line=ref('inspect'))
