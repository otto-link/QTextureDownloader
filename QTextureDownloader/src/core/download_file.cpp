/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General Public
   License. The full license is in the file LICENSE, distributed with this software. */
#include <QEventLoop>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QUrl>

#include "qtd/logger.hpp"

namespace qtd
{

bool download_file(const std::string &url, const std::string &file_path)
{
  QNetworkAccessManager manager;
  QNetworkRequest       request(QUrl(url.c_str()));
  QNetworkReply        *reply = manager.get(request);

  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec(); // wait until finished

  if (reply->error() != QNetworkReply::NoError)
  {
    QTD_LOG->error("download_file: download error");
    reply->deleteLater();
    return false;
  }

  QFile file(file_path.c_str());
  if (!file.open(QIODevice::WriteOnly))
  {
    QTD_LOG->error("download_file: error writing file: {}", file_path);
    reply->deleteLater();
    return false;
  }

  file.write(reply->readAll());
  file.close();
  reply->deleteLater();
  return true;
}

} // namespace qtd
