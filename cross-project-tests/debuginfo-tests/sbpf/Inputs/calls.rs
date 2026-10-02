#![no_std]
#[inline(never)]
#[no_mangle]
pub extern "C" fn inner(x: u64) -> u64 {
    unsafe { core::ptr::read_volatile(&x) }.wrapping_add(3)
}
#[inline(never)]
#[no_mangle]
pub extern "C" fn middle(x: u64) -> u64 {
    inner(x).wrapping_add(5)
}
#[no_mangle]
pub extern "C" fn entrypoint() -> u64 {
    middle(7).wrapping_add(7)
}
