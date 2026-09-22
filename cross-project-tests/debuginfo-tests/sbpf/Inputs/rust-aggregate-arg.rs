// Rust analogue of debuginfo-tests/dexter-tests/aggregate-indirect-arg.cpp.
// Exercise Rust by-value aggregate fields, not the C++ destructor/ABI rules.
#![no_std]

#[repr(C)]
pub struct SVal {
    data: u64,
    kind: u64,
}

#[inline(never)]
#[no_mangle]
pub fn bar(v: &SVal) -> u64 {
    unsafe { core::ptr::read_volatile(&v.kind) }
}

#[inline(never)]
#[no_mangle]
pub fn foo(v: SVal) -> u64 {
    bar(&v) // DexLabel('foo')
}

#[no_mangle]
pub extern "C" fn entrypoint() -> u64 {
    foo(SVal {
        data: 0,
        kind: 2142,
    })
}

// DexExpectWatchValue('v.data', '0', on_line=ref('foo'))
// DexExpectWatchValue('v.kind', '2142', on_line=ref('foo'))
