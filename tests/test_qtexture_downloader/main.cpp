/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General Public
   License. The full license is in the file LICENSE, distributed with this software. */
#include <QApplication>

#include "qtexture_downloader.hpp"

int main(int argc, char *argv[])
{
  QTD_LOG->info("testing qtexture_downloader...");

  qputenv("QT_LOGGING_RULES", QTD_QPUTENV_QT_LOGGING_RULES);
  QApplication app(argc, argv);

  qtd::TextureDownloader *dw = new qtd::TextureDownloader("Test widget");
  dw->show();

  // qtd::TextureManager tm;

  // // tm.update();
  // // tm.save();

  // tm.load();

  // tm.file_from("toto.json");
  // tm.file_to("toto.json");

  // qtd::Texture tex;
  // tex.from_poly_haven("rocky_terrain_02");

  return app.exec();
}
