"""Nox sessions."""

from __future__ import annotations

import os
import shutil

import nox

nox.needs_version = ">=2024.3.2"
nox.options.default_venv_backend = "uv"

nox.options.sessions = ["lint"]

PYTHON_ALL_VERSIONS = ["3.9", "3.10", "3.11", "3.12", "3.13"]

if os.environ.get("CI", None):
    nox.options.error_on_missing_interpreters = True


@nox.session(reuse_venv=True)
def lint(session: nox.Session) -> None:
    """Run the linter."""
    if shutil.which("pre-commit") is None:
        session.install("pre-commit")

    session.run("pre-commit", "run", "--all-files", *session.posargs, external=True)
