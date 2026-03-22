(function () {
  if (typeof Module === "undefined") { window.Module = {}; }
  Module.preRun = Module.preRun || [];
  Module.preRun.push(function () {
    function ensureDir(path) {
      if (!path || path === "/") return;
      try {
        if (!FS.analyzePath(path).exists) FS.mkdirTree(path);
      } catch (e) {}
    }
    ensureDir("/assets");
    FS.createPreloadedFile("/assets", "tile2.png", "assets/tile2.png", true, false);
    ensureDir("/assets");
    FS.createPreloadedFile("/assets", "tile24.png", "assets/tile24.png", true, false);
    ensureDir("/assets");
    FS.createPreloadedFile("/assets", "tile5.png", "assets/tile5.png", true, false);
    ensureDir("/scripts");
    FS.createPreloadedFile("/scripts", "main.bu", "scripts/main.bu", true, false);
    ensureDir("/scripts");
    FS.createPreloadedFile("/scripts", "main.buc", "scripts/main.buc", true, false);
  });
})();
