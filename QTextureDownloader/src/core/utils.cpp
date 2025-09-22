/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General Public
   License. The full license is in the file LICENSE, distributed with this software. */
#include <fstream>

#include <QBuffer>
#include <QByteArray>

#include "qtd/logger.hpp"
#include "qtd/utils.hpp"

namespace qtd
{

nlohmann::json json_from_file(const std::string &fname)
{
  nlohmann::json json;
  std::ifstream  file(fname);

  if (file.is_open())
  {
    file >> json;
    file.close();
    QTD_LOG->trace("json_from_file: JSON successfully loaded from {}", fname);
  }
  else
  {
    QTD_LOG->error("json_from_file: Could not open file {} to load JSON", fname);
  }

  return json;
}

void json_to_file(const nlohmann::json &json,
                  const std::string    &fname,
                  bool                  merge_with_existing_content)
{
  nlohmann::json final_json = json;

  if (merge_with_existing_content)
  {
    std::ifstream infile(fname);
    if (infile.is_open())
    {
      try
      {
        nlohmann::json existing;
        infile >> existing;
        infile.close();

        // Merge new JSON into existing JSON
        existing.merge_patch(json);
        final_json = existing;

        QTD_LOG->trace("json_to_file: merged JSON with existing content in {}", fname);
      }
      catch (const std::exception &e)
      {
        QTD_LOG->warn(
            "json_to_file: Could not parse existing JSON in {} ({}). Overwriting "
            "instead.",
            fname,
            e.what());
      }
    }
  }

  std::ofstream outfile(fname);
  if (outfile.is_open())
  {
    outfile << final_json.dump(4);
    outfile.close();
    QTD_LOG->trace("json_to_file: JSON successfully written to {}", fname);
  }
  else
  {
    QTD_LOG->error("json_to_file: Could not open file {} to save JSON", fname);
  }
}

std::vector<uint16_t> load_png_as_16bit_rgba(const std::string &path,
                                             int               &width,
                                             int               &height)
{
  QImage img(path.c_str());
  if (img.isNull())
  {
    return {};
  }

  // Convert image to 16-bit RGBA format (Qt ≥ 5.13)
  if (img.format() != QImage::Format_RGBA64)
    img = img.convertToFormat(QImage::Format_RGBA64);

  width = img.width();
  height = img.height();
  const int channel_count = 4; // R, G, B, A

  std::vector<uint16_t> data(static_cast<size_t>(width) * height * channel_count);

  for (int y = 0; y < height; ++y)
  {
    const uint16_t *scanline = reinterpret_cast<const uint16_t *>(img.constScanLine(y));
    std::memcpy(&data[y * width * channel_count],
                scanline,
                static_cast<size_t>(width) * channel_count * sizeof(uint16_t));
  }

  return data; // RVO handles return efficiently, no need for std::move
}

std::string qimage_to_base64(const QImage &img)
{
  if (img.isNull())
    return {};

  QByteArray bytes;
  QBuffer    buffer(&bytes);
  buffer.open(QIODevice::WriteOnly);
  img.save(&buffer, "PNG");

  return bytes.toBase64().toStdString();
}

QImage qimage_from_base64(const std::string &b64)
{
  if (b64.empty())
    return {};

  QByteArray data = QByteArray::fromBase64(QByteArray::fromStdString(b64));
  QImage     img;
  img.loadFromData(data, "PNG");
  return img;
}

} // namespace qtd
