#!/usr/bin/env python3
"""
Web build script (direct file compilation with Emscripten).

Builds:
  - libbu
  - graphics
  - main (output: main.html/.js/.wasm)
"""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parent
EXTERNAL_RAYLIB_SRC = ROOT / "external" / "raylib" / "src"
EXTERNAL_RAYLIB_WEB_LIB = EXTERNAL_RAYLIB_SRC / "libraylib.web.a"


@dataclass(frozen=True)
class SourceFile:
    module: str
    path: Path
    is_c: bool


def run_cmd(cmd: list[str], cwd: Path | None = None) -> None:
    print(f"[cmd] {' '.join(cmd)}")
    subprocess.run(cmd, cwd=cwd or ROOT, check=True)


def have_tool(name: str) -> bool:
    return shutil.which(name) is not None


def check_emscripten() -> None:
    required = ("emcc", "em++", "emmake")
    missing = [tool for tool in required if not have_tool(tool)]
    if missing:
        missing_str = ", ".join(missing)
        raise RuntimeError(
            f"Emscripten tools not found: {missing_str}. "
            "Load emsdk environment first."
        )


def ensure_raylib_web() -> Path:
    if EXTERNAL_RAYLIB_WEB_LIB.exists():
        return EXTERNAL_RAYLIB_WEB_LIB

    if EXTERNAL_RAYLIB_SRC.exists():
        run_cmd(["emmake", "make", "PLATFORM=PLATFORM_WEB", "-B"], cwd=EXTERNAL_RAYLIB_SRC)
        if EXTERNAL_RAYLIB_WEB_LIB.exists():
            return EXTERNAL_RAYLIB_WEB_LIB
        raise RuntimeError(f"raylib web library was not generated: {EXTERNAL_RAYLIB_WEB_LIB}")

    raise RuntimeError(
        "raylib source not found at external/raylib/src.\n"
        "Clone it first, for example:\n"
        "  git clone --depth 1 https://github.com/raysan5/raylib.git external/raylib"
    )


def gather_sources() -> list[SourceFile]:
    sources: list[SourceFile] = []

    libbu_dir = ROOT / "libbu" / "libbu" / "src"
    for src in sorted(libbu_dir.glob("*.cpp")):
        sources.append(SourceFile("libbu", src, False))
    for src in sorted(libbu_dir.glob("*.c")):
        sources.append(SourceFile("libbu", src, True))
    for src in sorted((libbu_dir / "miniz").glob("*.c")):
        sources.append(SourceFile("libbu", src, True))

    for src in sorted((ROOT / "graphics" / "src").glob("*.cpp")):
        sources.append(SourceFile("graphics", src, False))

    for src in sorted((ROOT / "main" / "src").glob("*.cpp")):
        sources.append(SourceFile("main", src, False))

    if not sources:
        raise RuntimeError("No source files found for libbu/graphics/main.")
    return sources


def include_flags() -> list[str]:
    include_dirs = [
        ROOT / "libbu" / "libbu" / "include",
        ROOT / "libbu" / "libbu" / "src",
        ROOT / "libbu" / "libbu" / "src" / "miniz",
        ROOT / "graphics" / "src",
        ROOT / "main" / "src",
        EXTERNAL_RAYLIB_SRC,
    ]
    return [f"-I{inc}" for inc in include_dirs if inc.exists()]


def mode_compile_flags(build_type: str, is_c: bool) -> list[str]:
    flags = ["-DPLATFORM_WEB"]
    flags.append("-std=c11" if is_c else "-std=c++17")

    if build_type == "release":
        flags.extend(["-O3", "-DNDEBUG"])
    else:
        flags.extend(["-O1", "-g", "-DDEBUG", "-D_DEBUG"])
    return flags


def mode_link_flags(build_type: str) -> list[str]:
    common = [
        "-s", "USE_GLFW=3",
        "-s", "ALLOW_MEMORY_GROWTH=1",
        "-s", "STACK_SIZE=5242880",
        "-s", "FORCE_FILESYSTEM=1",
        "-s", "EXPORTED_RUNTIME_METHODS=['ccall','cwrap']",
        "-s", "EXPORTED_FUNCTIONS=['_main']",
    ]

    if build_type == "release":
        return ["-O3", "-DNDEBUG"] + common + ["-s", "ASSERTIONS=0"]
    return ["-O1", "-g"] + common + ["-s", "ASSERTIONS=2", "-s", "SAFE_HEAP=1"]


def object_path(build_dir: Path, source: SourceFile) -> Path:
    obj_dir = build_dir / "obj" / source.module
    obj_dir.mkdir(parents=True, exist_ok=True)
    stem = source.path.stem + ".o"
    return obj_dir / stem


def needs_rebuild(src: Path, obj: Path) -> bool:
    if not obj.exists():
        return True
    return src.stat().st_mtime > obj.stat().st_mtime


def compile_one(source: SourceFile, build_dir: Path, inc_flags: list[str], build_type: str) -> tuple[Path, bool]:
    obj = object_path(build_dir, source)
    if not needs_rebuild(source.path, obj):
        return obj, False

    compiler = "emcc" if source.is_c else "em++"
    cmd = [
        compiler,
        *mode_compile_flags(build_type, source.is_c),
        *inc_flags,
        "-c",
        str(source.path),
        "-o",
        str(obj),
    ]
    run_cmd(cmd)
    return obj, True


def compile_all(sources: list[SourceFile], build_dir: Path, jobs: int, build_type: str) -> list[Path]:
    inc_flags = include_flags()
    objects: list[Path] = []
    compiled = 0
    cached = 0

    with ThreadPoolExecutor(max_workers=max(1, jobs)) as pool:
        futures = {
            pool.submit(compile_one, src, build_dir, inc_flags, build_type): src
            for src in sources
        }
        for fut in as_completed(futures):
            src = futures[fut]
            obj, did_compile = fut.result()
            objects.append(obj)
            if did_compile:
                compiled += 1
                print(f"[ok] compiled {src.path}")
            else:
                cached += 1
                print(f"[ok] cached   {src.path}")

    print(f"[info] compile summary: compiled={compiled}, cached={cached}, total={len(sources)}")
    return sorted(objects)


def link_web(objects: list[Path], raylib_lib: Path, build_dir: Path, build_type: str) -> Path:
    output_html = build_dir / ("main.html" if build_type == "debug" else "main.release.html")
    cmd = [
        "em++",
        *(str(obj) for obj in objects),
        str(raylib_lib),
        *mode_link_flags(build_type),
    ]

    assets_dir = ROOT / "bin" / "assets"
    if assets_dir.exists():
        cmd.extend(["--preload-file", f"{assets_dir}@/assets"])

    shell_file = ROOT / "shell.html"
    if shell_file.exists():
        cmd.extend(["--shell-file", str(shell_file)])

    cmd.extend(["-o", str(output_html)])
    run_cmd(cmd)
    return output_html


def run_server(build_dir: Path, output_html: Path) -> None:
    print(f"[info] serving {build_dir} at http://localhost:8000/{output_html.name}")
    run_cmd(["python3", "-m", "http.server", "8000"], cwd=build_dir)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build BuGame for Web (direct file compilation, no CMake)."
    )
    parser.add_argument(
        "--release",
        action="store_true",
        help="Build in release mode (default is debug).",
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="Delete build/web before build.",
    )
    parser.add_argument(
        "--jobs",
        type=int,
        default=max(1, os.cpu_count() or 1),
        help="Parallel compile jobs.",
    )
    parser.add_argument(
        "--run",
        action="store_true",
        help="Run simple web server after build.",
    )
    parser.add_argument(
        "--info",
        action="store_true",
        help="Print effective configuration and exit.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    build_type = "release" if args.release else "debug"
    build_dir = ROOT / "build" / "web"

    if args.info:
        print(f"root: {ROOT}")
        print(f"build_dir: {build_dir}")
        print(f"build_type: {build_type}")
        print(f"raylib_src: {EXTERNAL_RAYLIB_SRC}")
        print(f"raylib_web_lib: {EXTERNAL_RAYLIB_WEB_LIB}")
        return 0

    if args.clean and build_dir.exists():
        print(f"[info] removing {build_dir}")
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)

    check_emscripten()
    raylib_lib = ensure_raylib_web()
    sources = gather_sources()
    objects = compile_all(sources, build_dir, args.jobs, build_type)
    output_html = link_web(objects, raylib_lib, build_dir, build_type)

    print(f"[ok] web build complete: {output_html}")
    if args.run:
        run_server(build_dir, output_html)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as exc:
        print(f"[error] command failed with exit code {exc.returncode}")
        raise SystemExit(exc.returncode)
    except RuntimeError as exc:
        print(f"[error] {exc}")
        raise SystemExit(1)
