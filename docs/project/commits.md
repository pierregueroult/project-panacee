# Conventional Commits Guide

This document defines the commit message convention used in this project.

## Normative keywords (RFC 2119)

The key words **MUST**, **MUST NOT**, **REQUIRED**, **SHALL**, **SHALL NOT**, **SHOULD**, **SHOULD NOT**, **RECOMMENDED**, **MAY**, and **OPTIONAL** in this document are to be interpreted as described in RFC 2119.

- **MUST / REQUIRED / SHALL**: absolute requirement.
- **MUST NOT / SHALL NOT**: absolute prohibition.
- **SHOULD / RECOMMENDED**: valid reasons may exist to ignore, but implications must be understood.
- **SHOULD NOT / NOT RECOMMENDED**: generally discouraged, but may be acceptable in specific cases with careful consideration.
- **MAY / OPTIONAL**: truly optional.

---

## Commit message structure

A commit message **MUST** use the following structure:

<type>[optional scope][optional !]: <description>

[optional body]

[optional footer(s)]

### Required rules

1. A commit **MUST** be prefixed with a `type` (a noun such as `feat`, `fix`, etc.).
2. A `scope` **MAY** be provided after the type.
3. A `!` **MAY** be provided before `:` to indicate a breaking change.
4. The prefix **MUST** end with a terminal `: ` (colon + space).
5. A short `description` **MUST** immediately follow the prefix.

Valid examples:
- `feat: add SSO authentication`
- `fix(parser): handle multiple spaces in arrays`
- `feat(api)!: remove legacy v1 endpoint`

---

## Commit types

### Required type usage

- `feat` **MUST** be used when a commit adds a new feature to the application or library.
- `fix` **MUST** be used when a commit represents a bug fix.

### Additional types

Types other than `feat` and `fix` **MAY** be used, for example:
- `docs`
- `refactor`
- `test`
- `chore`
- `build`
- `ci`
- `perf`
- `style`

Example:
- `docs: update API reference`

---

## Scope

A scope is optional but often useful.

- A scope **MAY** be provided after the type.
- A scope **MUST** be a noun describing a section of the codebase, surrounded by parentheses.

Examples:
- `fix(parser): ...`
- `feat(auth): ...`
- `chore(deps): ...`

---

## Description

- The description **MUST** be a short summary of the code changes.
- It **MUST** appear immediately after `: `.

Example:
- `fix: prevent startup crash on iOS`

---

## Body

- A longer commit body **MAY** be provided after the short description to add context.
- The body **MUST** begin one blank line after the description.
- The body is free-form and **MAY** contain any number of newline-separated paragraphs.

Example:

fix(cache): prevent cache corruption under concurrent writes

Add optimistic locking around write operations.
The issue primarily appeared under high load.

---

## Footers

- One or more footers **MAY** be provided one blank line after the body.
- If there is no body, footers **MAY** appear one blank line after the description.
- Each footer **MUST** consist of:
  - a token,
  - followed by either `: ` or ` #`,
  - followed by a string value.

Examples:
- `Refs: #123`
- `Acked-by: Jane Doe <jane@example.com>`
- `Reviewed-by: Backend Team`

### Footer token rules

- A footer token **MUST** use `-` instead of whitespace (e.g., `Acked-by`).
- An exception is made for `BREAKING CHANGE`, which **MAY** also be used as a token.
- `BREAKING-CHANGE` **MUST** be treated as synonymous with `BREAKING CHANGE` in footers.

---

## Breaking changes

Breaking changes **MUST** be indicated in one of the following ways:

1. In the type/scope prefix with `!`  
   Example: `feat(api)!: remove /v1/users endpoint`

2. In a footer using `BREAKING CHANGE: <description>`  
   Example: `BREAKING CHANGE: environment variables now take precedence over config files`

Additional rules:
- If included as a footer, `BREAKING CHANGE` **MUST** be uppercase.
- If `!` is used in the prefix, the `BREAKING CHANGE:` footer **MAY** be omitted.
- When `!` is used and no breaking footer is present, the commit description **SHALL** describe the breaking change.

---

## Case sensitivity

The units of information in Conventional Commits **MUST NOT** be treated as case-sensitive by implementors, with the exception of `BREAKING CHANGE`, which **MUST** be uppercase when used as that token.

---

## Full examples

### 1) Feature commit

feat(auth): add OAuth2 login flow

### 2) Fix with body and footer

fix(parser): handle arrays containing extra spaces

The parser was collapsing significant spaces in quoted strings.
Behavior is now aligned with the internal JSON parsing specification.

Refs: #456
Acked-by: Marie Curie <marie@example.com>

### 3) Breaking change via prefix

feat(config)!: switch default format from YAML to TOML

### 4) Breaking change via footer

refactor(core): simplify module loading pipeline

BREAKING CHANGE: plugin initialization order changed; plugins must now declare explicit dependencies.

---

## Recommended best practices

- Use imperative mood in descriptions (`add`, `fix`, `remove`).
- Keep the description concise (ideally <= 72 characters).
- Use scope when it improves clarity.
- Avoid vague messages (`update`, `fix stuff`).