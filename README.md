# fast-json

A json-library that is _somewhat_ convenient, fast to build and runs fast.


## Parsing

`json::parse_json(text)` returns a `JsonRoot`. Input that is not JSON throws
`json::ParseError`, a `std::runtime_error` whose `what()` ends with
`(line L, column C)`; `message()`, `line()` and `column()` give the parts. This
covers truncated input (inside an object, array or string, or after a comma), a
missing comma or colon (`{"a":1 "b":2}`, `[1 2]`), a doubled or leading comma,
mismatched brackets, malformed numbers and text after the document. A comma
before a closing `]` or `}` is accepted on purpose.

### Numbers

A number node keeps the text as written (`-1.5e-3`); the conversion happens
when it is read with `number<T>()` (or `as<T>()`, `get_to`):

- Syntax: `-?digits`, optionally `.digits`, optionally `e` or `E` with an
  optional sign and digits. No leading `+`, no leading zeros (`01`), no `1.`
  or `.5`.
- An integer type accepts only integer notation. `"12"` and `"-3"` work;
  `"1.5"` and `"2e3"` throw `std::invalid_argument` instead of being cut to
  `1` and `2`, and a negative number into an unsigned type throws too.
- A floating point type accepts every form: `1.5`, `2e3`, `1E-2`, `-1.5e-3`,
  and integers.
- `dump()` prints a number as written.
