/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General Public
   License. The full license is in the file LICENSE, distributed with this software. */
#include <filesystem>

#include "qtd/config.hpp"
#include "qtd/json_fetcher.hpp"
#include "qtd/logger.hpp"
#include "qtd/texture_manager.hpp"
#include "qtd/utils.hpp"

namespace qtd
{

TextureManager::TextureManager(const std::string &storage_path_)
    : storage_path(storage_path_)
{
  QTD_LOG->trace("TextureManager::TextureManager");

  // create storage
  std::filesystem::path dir = std::filesystem::path(this->storage_path);
  if (!std::filesystem::exists(dir))
  {
    QTD_LOG->info("TextureManager::TextureManager: creating storage repertory {}",
                  dir.string());
    std::filesystem::create_directories(dir);
  }
}

void TextureManager::file_from(const std::string &fname)
{
  nlohmann::json json = json_from_file(fname);
  this->json_from(json);
}

void TextureManager::file_to(const std::string &fname) const
{
  json_to_file(this->json_to(), fname);
}

std::map<std::string, Texture> &TextureManager::get_textures() { return this->textures; }

void TextureManager::json_from(nlohmann::json const &j)
{
  this->textures.clear();

  for (auto &[key, value] : j.items())
  {
    this->textures[key] = Texture();
    this->textures[key].json_from(value);
  }
}

nlohmann::json TextureManager::json_to() const
{
  nlohmann::json json;

  for (auto &[key, tex] : this->textures)
    json[key] = tex.json_to();

  return json;
}

void TextureManager::load() { this->file_from(this->storage_path + "/db.json"); }

void TextureManager::save() const { this->file_to(this->storage_path + "/db.json"); }

void TextureManager::update()
{
  QTD_LOG->trace("TextureManager::update");
  this->update_from_poly_haven();
}

void TextureManager::update_from_poly_haven()
{
  QTD_LOG->trace("TextureManager::update_from_poly_haven");

  // --- retrieve asset list

  std::vector<std::string> asset_ids;

  JsonFetcher    json_fetcher;
  nlohmann::json json_asset_list = json_fetcher.fetch_sync(
      "https://api.polyhaven.com/assets?type=textures");

  if (json_asset_list.empty())
  {
    QTD_LOG->error("TextureManager::update_from_poly_haven: could not fetch asset list");
    return;
  }

  for (auto &e : json_asset_list.items())
    asset_ids.push_back(e.key());

  // --- update textures data

  int k = 0; // TODO DBG

  for (auto &source_id : asset_ids)
  {
    // build up a unique ID based on the source and the source ID
    const std::string id = "PolyHaven_" + source_id;

    QTD_LOG->debug("{}", id);

    // create and/or replace
    Texture new_texture = Texture();
    new_texture.set_id(id);
    bool ok = new_texture.from_poly_haven(source_id, json_asset_list);

    if (ok)
      this->textures[id] = new_texture;

    // TODO TEST
    k++;
    if (k > 3)
      break;
  }
}

} // namespace qtd
