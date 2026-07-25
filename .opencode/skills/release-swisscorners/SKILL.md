---
name: release-swisscorners
description: Tag a release, bump version, generate notes, and instruct on publishing to the Rebble store
license: MIT
---

## What I do

Given a new version string (e.g. `1.2.0`), I:

1. Bump `"version"` in `package.json` and `package-lock.json`
2. Collect all commit messages since the last tag using `git log`
3. Categorize them as **New features**, **Bug fixes**, **Improvements**, or **Developer**
4. Stage all changes, commit with `"bump to v<VERSION>"`, and create a signed/annotated git tag `v<VERSION>`
5. Print the final release notes and the `pebble publish` command for the user to run

## When to use me

Use this when you are cutting a new Pebble watchface release for the Rebble store.

## How to use

Tell me the target version (e.g. `1.2.0`). I will bump, tag, generate release notes, and then tell you to run:

```
pebble login
pebble publish --release-notes "<NOTES>" --is-published
```
