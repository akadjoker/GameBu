#!/usr/bin/env python3
"""
BuGame multi-platform build script.

Platforms:
  - web:     Emscripten direct compile (outputs HTML/JS/WASM)
  - android: CMake+NDK native build (no APK packaging)

Examples:
  python3 build.py --release
  python3 build.py web --run
  python3 build.py android --ndk ~/Android/Sdk/ndk/27.0.12077973 --abi arm64-v8a --release
"""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parent

# Raylib source used for WEB static library build.
EXTERNAL_RAYLIB_SRC = ROOT / "external" / "raylib" / "src"
EXTERNAL_RAYLIB_WEB_LIB = EXTERNAL_RAYLIB_SRC / "libraylib.web.a"

# libbu layout changed over time; support both.
LIBBU_SRC_CANDIDATES = [
    ROOT / "libbu" / "src",
    ROOT / "libbu" / "libbu" / "src",
]
LIBBU_INCLUDE_CANDIDATES = [
    ROOT / "libbu" / "include",
    ROOT / "libbu" / "libbu" / "include",
]

VENDOR_MINIZ_DIR = ROOT / "vendor" / "miniz"
VENDOR_BOX2D_SRC = ROOT / "vendor" / "box2d" / "src"
VENDOR_BOX2D_INCLUDE = ROOT / "vendor" / "box2d" / "include"


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


def first_existing(paths: list[Path], label: str) -> Path:
    for path in paths:
        if path.exists():
            return path
    raise RuntimeError(f"Could not find {label}. Tried: {', '.join(str(p) for p in paths)}")


def libbu_src_dir() -> Path:
    return first_existing(LIBBU_SRC_CANDIDATES, "libbu src directory")


def libbu_include_dir() -> Path:
    return first_existing(LIBBU_INCLUDE_CANDIDATES, "libbu include directory")


def check_emscripten() -> None:
    required = ("emcc", "em++", "emmake")
    missing = [tool for tool in required if not have_tool(tool)]
    if missing:
        raise RuntimeError(
            f"Emscripten tools not found: {', '.join(missing)}. Load emsdk environment first."
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


def gather_sources_web() -> list[SourceFile]:
    sources: list[SourceFile] = []

    lib_src = libbu_src_dir()
    for src in sorted(lib_src.glob("*.cpp")):
        sources.append(SourceFile("libbu", src, False))
    for src in sorted(lib_src.glob("*.c")):
        sources.append(SourceFile("libbu", src, True))

    if VENDOR_MINIZ_DIR.exists():
        for src in sorted(VENDOR_MINIZ_DIR.glob("*.c")):
            sources.append(SourceFile("miniz", src, True))

    for src in sorted((ROOT / "graphics" / "src").glob("*.cpp")):
        sources.append(SourceFile("graphics", src, False))

    for src in sorted((ROOT / "main" / "src").glob("*.cpp")):
        sources.append(SourceFile("main", src, False))

    if VENDOR_BOX2D_SRC.exists():
        for src in sorted(VENDOR_BOX2D_SRC.rglob("*.cpp")):
            sources.append(SourceFile("box2d", src, False))

    if not sources:
        raise RuntimeError("No source files found for web build.")
    return sources


def include_flags_web() -> list[str]:
    include_dirs = [
        libbu_include_dir(),
        libbu_src_dir(),
        ROOT / "graphics" / "src",
        ROOT / "main" / "src",
        EXTERNAL_RAYLIB_SRC,
        VENDOR_MINIZ_DIR,
        VENDOR_BOX2D_INCLUDE,
        VENDOR_BOX2D_SRC,
    ]
    return [f"-I{inc}" for inc in include_dirs if inc.exists()]


def compile_flags_web(build_type: str, is_c: bool) -> list[str]:
    flags = ["-DPLATFORM_WEB"]
    flags.append("-std=c11" if is_c else "-std=c++17")

    if build_type == "release":
        flags.extend(["-O3", "-DNDEBUG"])
    else:
        flags.extend(["-O1", "-g", "-DDEBUG", "-D_DEBUG"])
    return flags


def link_flags_web(build_type: str) -> list[str]:
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

    rel = source.path.relative_to(ROOT).with_suffix("")
    safe_name = str(rel).replace(os.sep, "__").replace(":", "_") + ".o"
    return obj_dir / safe_name


def needs_rebuild(src: Path, obj: Path) -> bool:
    if not obj.exists():
        return True
    return src.stat().st_mtime > obj.stat().st_mtime


def compile_one_web(source: SourceFile, build_dir: Path, inc_flags: list[str], build_type: str) -> tuple[Path, bool]:
    obj = object_path(build_dir, source)
    if not needs_rebuild(source.path, obj):
        return obj, False

    compiler = "emcc" if source.is_c else "em++"
    cmd = [
        compiler,
        *compile_flags_web(build_type, source.is_c),
        *inc_flags,
        "-c",
        str(source.path),
        "-o",
        str(obj),
    ]
    run_cmd(cmd)
    return obj, True


def compile_all_web(sources: list[SourceFile], build_dir: Path, jobs: int, build_type: str) -> list[Path]:
    inc_flags = include_flags_web()
    objects: list[Path] = []
    compiled = 0
    cached = 0

    with ThreadPoolExecutor(max_workers=max(1, jobs)) as pool:
        futures = {
            pool.submit(compile_one_web, src, build_dir, inc_flags, build_type): src
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
        *link_flags_web(build_type),
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


def build_web(args: argparse.Namespace) -> int:
    build_type = "release" if args.release else "debug"
    build_dir = ROOT / "build" / "web"

    if args.info:
        print(f"platform: web")
        print(f"root: {ROOT}")
        print(f"build_dir: {build_dir}")
        print(f"build_type: {build_type}")
        print(f"raylib_src: {EXTERNAL_RAYLIB_SRC}")
        print(f"raylib_web_lib: {EXTERNAL_RAYLIB_WEB_LIB}")
        print(f"libbu_src: {libbu_src_dir()}")
        print(f"libbu_include: {libbu_include_dir()}")
        return 0

    if args.clean and build_dir.exists():
        print(f"[info] removing {build_dir}")
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)

    check_emscripten()
    raylib_lib = ensure_raylib_web()
    sources = gather_sources_web()
    objects = compile_all_web(sources, build_dir, args.jobs, build_type)
    output_html = link_web(objects, raylib_lib, build_dir, build_type)

    print(f"[ok] web build complete: {output_html}")
    if args.run:
        run_server(build_dir, output_html)
    return 0


def detect_ndk(ndk_arg: str | None) -> Path:
    if ndk_arg:
        ndk = Path(ndk_arg).expanduser().resolve()
    else:
        env_ndk = (
            os.environ.get("ANDROID_NDK_HOME")
            or os.environ.get("ANDROID_NDK_ROOT")
            or os.environ.get("NDK_HOME")
        )
        if not env_ndk:
            raise RuntimeError(
                "Android NDK not set. Use --ndk or set ANDROID_NDK_HOME/ANDROID_NDK_ROOT."
            )
        ndk = Path(env_ndk).expanduser().resolve()

    if not ndk.exists():
        raise RuntimeError(f"Android NDK path does not exist: {ndk}")
    return ndk


def build_android(args: argparse.Namespace) -> int:
    if not have_tool("cmake"):
        raise RuntimeError("cmake not found in PATH")

    build_type = "Release" if args.release else "Debug"
    build_dir = ROOT / "build" / "android" / f"{args.abi}-{build_type.lower()}"

    if args.info:
        ndk_hint = (
            args.ndk
            or os.environ.get("ANDROID_NDK_HOME")
            or os.environ.get("ANDROID_NDK_ROOT")
            or os.environ.get("NDK_HOME")
            or "<not set>"
        )
        toolchain_hint = (
            str(Path(ndk_hint).expanduser().resolve() / "build" / "cmake" / "android.toolchain.cmake")
            if ndk_hint != "<not set>"
            else "<not set>"
        )
        print("platform: android")
        print(f"root: {ROOT}")
        print(f"ndk: {ndk_hint}")
        print(f"toolchain: {toolchain_hint}")
        print(f"abi: {args.abi}")
        print(f"api: android-{args.api}")
        print(f"build_type: {build_type}")
        print(f"build_dir: {build_dir}")
        print(f"generator: {args.generator}")
        print(f"target: {args.target}")
        if args.raylib_include_dir:
            print(f"raylib_include_dir: {args.raylib_include_dir}")
        if args.raylib_lib_dir:
            print(f"raylib_lib_dir: {args.raylib_lib_dir}")
        return 0

    ndk = detect_ndk(args.ndk)
    toolchain = ndk / "build" / "cmake" / "android.toolchain.cmake"
    if not toolchain.exists():
        raise RuntimeError(f"Android toolchain file not found: {toolchain}")

    if args.clean and build_dir.exists():
        print(f"[info] removing {build_dir}")
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)

    configure_cmd = [
        "cmake",
        "-S", str(ROOT),
        "-B", str(build_dir),
        "-G", args.generator,
        f"-DCMAKE_TOOLCHAIN_FILE={toolchain}",
        f"-DANDROID_ABI={args.abi}",
        f"-DANDROID_PLATFORM=android-{args.api}",
        "-DANDROID_STL=c++_static",
        f"-DCMAKE_BUILD_TYPE={build_type}",
    ]

    if args.raylib_include_dir:
        configure_cmd.append(f"-DCMAKE_INCLUDE_PATH={Path(args.raylib_include_dir).expanduser().resolve()}")
    if args.raylib_lib_dir:
        configure_cmd.append(f"-DCMAKE_LIBRARY_PATH={Path(args.raylib_lib_dir).expanduser().resolve()}")

    for extra in args.cmake_arg:
        configure_cmd.append(extra)

    run_cmd(configure_cmd)

    build_cmd = [
        "cmake",
        "--build", str(build_dir),
        "--target", args.target,
        "--parallel", str(max(1, args.jobs)),
    ]
    run_cmd(build_cmd)

    print(f"[ok] android native build complete: {build_dir}")
    print("[info] this step builds native binaries only (APK/Gradle packaging is separate).")
    return 0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build BuGame for Web (Emscripten) or Android (NDK/CMake)."
    )
    parser.add_argument(
        "platform",
        nargs="?",
        choices=("web", "android"),
        default="web",
        help="Target platform (default: web).",
    )

    parser.add_argument(
        "--release",
        action="store_true",
        help="Build in release mode (default is debug).",
    )
    parser.add_argument(
        "--clean",
        action="store_true",
        help="Delete target build folder before build.",
    )
    parser.add_argument(
        "--jobs",
        type=int,
        default=max(1, os.cpu_count() or 1),
        help="Parallel compile jobs.",
    )
    parser.add_argument(
        "--info",
        action="store_true",
        help="Print effective configuration and exit.",
    )

    # Web-only convenience.
    parser.add_argument(
        "--run",
        action="store_true",
        help="(web only) Run simple HTTP server after build.",
    )

    # Android options.
    parser.add_argument("--ndk", help="Android NDK root path.")
    parser.add_argument(
        "--abi",
        default="arm64-v8a",
        choices=("arm64-v8a", "armeabi-v7a", "x86", "x86_64"),
        help="Android ABI (default: arm64-v8a).",
    )
    parser.add_argument(
        "--api",
        type=int,
        default=24,
        help="Android API level (default: 24).",
    )
    parser.add_argument(
        "--generator",
        default="Ninja",
        help="CMake generator for android builds (default: Ninja).",
    )
    parser.add_argument(
        "--target",
        default="main",
        help="CMake target to build for android (default: main).",
    )
    parser.add_argument(
        "--cmake-arg",
        action="append",
        default=[],
        help="Extra raw argument passed to CMake configure (repeatable).",
    )
    parser.add_argument(
        "--raylib-include-dir",
        help="Optional extra include path where raylib headers are found.",
    )
    parser.add_argument(
        "--raylib-lib-dir",
        help="Optional extra library path where libraylib.* is found.",
    )

    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.platform == "android":
        return build_android(args)
    return build_web(args)


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as exc:
        print(f"[error] command failed with exit code {exc.returncode}")
        raise SystemExit(exc.returncode)
    except RuntimeError as exc:
        print(f"[error] {exc}")
        raise SystemExit(1)
