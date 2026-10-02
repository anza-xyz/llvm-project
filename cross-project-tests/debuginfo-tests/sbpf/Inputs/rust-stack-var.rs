// Rust port of debuginfo-tests/dexter-tests/stack-var.c.
#![no_std]

#[inline(never)]
#[no_mangle]
pub fn bar(test: &mut u64) {
    unsafe {
        core::ptr::read_volatile(test);
    }
}

#[no_mangle]
pub extern "C" fn entrypoint() -> u64 {
    let mut test = 23;
    bar(&mut test); // DexLabel('before_bar')
    unsafe { core::ptr::read_volatile(&test) } // DexLabel('after_bar')
}

// DexExpectWatchValue('test', '23', on_line=ref('before_bar'))
// DexExpectWatchValue('test', '23', on_line=ref('after_bar'))
