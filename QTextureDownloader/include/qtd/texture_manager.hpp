/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General Public
   License. The full license is in the file LICENSE, distributed with this software. */
#pragma once

#include "qtd/texture.hpp"

namespace qtd
{

class TextureManager
{
public:
  explicit TextureManager(const std::string &storage_path_ = "texture_downloader");

  std::map<std::string, Texture> &get_textures();

  std::vector<uint16_t> get_texture_rgba_16bit(const std::string &id,
                                               const TextureType &texture_type,
                                               const TextureRes  &texture_res);

  void load();
  void save() const;

  void update();
  void update_from_poly_haven();

private:
  void           file_from(const std::string &fname);
  void           file_to(const std::string &fname) const;
  void           json_from(nlohmann::json const &json);
  nlohmann::json json_to() const;

  // --- Members
  std::string                    storage_path;
  std::map<std::string, Texture> textures;
};

} // namespace qtd