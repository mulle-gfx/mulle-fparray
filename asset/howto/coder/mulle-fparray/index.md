# mulle-fparray — coder howto

<!-- Keywords: floatarray, dynamic-array, allocator, enumeration, sentinel -->

mulle-fparray provides a dynamic float array type used by animation code in
MulleCA. Two API tiers share the same memory layout — the base
`mulle__floatarray` (double underscore) and the convenience wrapper
`mulle_floatarray` (single underscore).

## Understand first

```bash
mulle-sde api cat mulle__floatarray
mulle-sde api cat mulle_floatarray
mulle-sde howto show --keyword howto coder
```

## Local references

| What | Path |
|------|------|
| Umbrella header | `src/mulle-fparray.h` |
| Base implementation | `src/float/mulle--floatarray.h`, `src/float/mulle--floatarray.c` |
| Wrapper implementation | `src/float/mulle-floatarray.h` |
| API doc (TOC) | `asset/dox/TOC.md` |
| Tests | `test/10-float/floatarray.c`, `test/10-float/floatarray-copy.c` |

## API families

- **Base (`mulle__floatarray`)** — explicit allocator parameter on every
  allocating operation. Defined in `src/float/mulle--floatarray.h`.
- **Wrapper (`mulle_floatarray`)** — embeds allocator in struct, omits
  allocator parameter. Defined in `src/float/mulle-floatarray.h`.

Both share `MULLE__FLOATARRAY_BASE` (four float pointers: `_storage`, `_curr`,
`_sentinel`, `_initial_storage`), so the structs are castable.

## When to use each tier

| Scenario | API | Reason |
|----------|-----|--------|
| Quick scripting / most code | `mulle_floatarray` | Embedded allocator, fewer parameters |
| Custom allocator control | `mulle__floatarray` | Explicit allocator per call |
| NULL-guarded safety | unprefixed (`mulle_floatarray_init`) | Returns NAN / 0 for NULL arrays |
| Hot loop performance | `_`-prefixed (`_mulle_floatarray_add`) | Skips NULL checks; caller guarantees non-NULL |
| Zero values / NAN values | `_mulle_floatarray_add` | `mulle_floatarray_add` silently drops 0.0 |

## Verify

Tests run via `mulle-sde test` — each `.c` file in `test/` is compiled and
linked against the library. Leaks are detected by `mulle-testallocator`.
