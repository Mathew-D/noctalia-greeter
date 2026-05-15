# noctalia-greeter TODO

## Display Manager Core

- Implement the real DRM/KMS + GBM backend for non-debug mode.
- Add libinput input handling for the DRM/KMS backend.
- Switch the acquired VT into graphics mode and restore text mode on exit.
- Handle compositor/session process lifetime and greeter shutdown cleanly after handoff.
- Decide whether failed user sessions should return to the greeter or exit.

## Authentication And Sessions

- Keep the PAM handle open through session launch so `pam_close_session` can run correctly.
- Add PAM conversation support for non-password prompts and expired password flows.
- Sanitize `.desktop` `Exec=` commands according to desktop-entry field-code rules.
- Add session filtering and ordering that matches common display managers.
- Add better error reporting for missing PAM service files, missing sessions, and disabled users.

## UI And Rendering

- Replace immediate-mode UI state with shell-like scene nodes and input areas.
- Add proper popup clipping, keyboard navigation, and scrollable session/user menus.
- Cache Pango/Cairo text textures instead of regenerating every frame.
- Add a real cursor/caret, password reveal toggle, caps-lock indicator, and loading state.
- Support HiDPI/content scaling consistently across debug and DRM backends.
- Load Noctalia palette/assets through a small resource-path helper instead of cwd-relative paths.

## Packaging And Void Linux

- Add a production runit service file.
- Add install docs once the DRM/KMS backend is usable.
- Add a Void package template.
- Document required groups/capabilities or setuid/polkit strategy for VT/DRM/session handling.
- Add rollback and recovery notes for testing on a real TTY.

## Quality

- Add unit tests for session discovery and desktop-entry parsing.
- Add tests for user enumeration filtering.
- Add a debug-mode screenshot smoke test.
- Add clang-format config matching `noctalia-shell`.
- Add CI build coverage for Meson debug and release builds.
