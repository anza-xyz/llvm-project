#![no_std]
#[inline(never)]
#[no_mangle]
pub extern "C" fn recurse(n: u64) -> u64 {
    let n = unsafe { core::ptr::read_volatile(&n) };
    if n == 0 {
        return 1;
    }
    let result = recurse(n - 1);
    unsafe { core::ptr::read_volatile(&result) }.wrapping_add(n)
}
#[no_mangle]
pub extern "C" fn entrypoint() -> u64 {
    recurse(3)
}
