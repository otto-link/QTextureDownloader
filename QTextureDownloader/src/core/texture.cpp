/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General Public
   License. The full license is in the file LICENSE, distributed with this software. */
#include "qtd/texture.hpp"
#include "qtd/config.hpp"
#include "qtd/image_fetcher.hpp"
#include "qtd/json_fetcher.hpp"
#include "qtd/logger.hpp"
#include "qtd/utils.hpp"

namespace qtd
{

TextureRes helper_poly_haven_res_converter(const std::string &text)
{
  if (text == "1k")
    return TextureRes::R1K;
  else if (text == "2k")
    return TextureRes::R2K;
  else if (text == "4k")
    return TextureRes::R4K;
  else if (text == "8k")
    return TextureRes::R8K;
  else
    return TextureRes::RUNKNOWN;
}

bool Texture::from_poly_haven(const std::string    &asset_id,
                              const nlohmann::json &json_asset_list)
{

  QTD_LOG->debug("Texture::from_poly_haven: {}", asset_id);

  bool ret = true; // means ok so far

  // base data
  if (!json_asset_list.contains(asset_id))
  {
    QTD_LOG->error("Texture::from_poly_haven: info - JSON parse error, asset {}",
                   asset_id);
    return false;
  }

  nlohmann::json j = json_asset_list[asset_id];

  this->source = "PolyHaven";
  this->id_from_source = asset_id;

  ret |= json_safe_get(j, "name", this->name);
  ret |= json_safe_get(j, "thumbnail_url", this->thumbnail_url);
  ret |= json_safe_get(j, "tags", this->tags);
  ret |= json_safe_get(j, "max_resolution", this->max_resolution);

  // thumbnail
  {
    ImageFetcher image_fetcher;
    this->thumbnail = image_fetcher.fetch_sync(this->thumbnail_url);

    if (this->thumbnail.isNull())
    {
      QTD_LOG->error("Texture::from_poly_haven: thumbnail download error, asset {}",
                     asset_id);
      return false;
    }
  }

  // texture files
  {
    JsonFetcher    json_fetcher;
    nlohmann::json j = json_fetcher.fetch_sync("https://api.polyhaven.com/files/" +
                                               asset_id);

    if (j.empty())
    {
      QTD_LOG->error(
          "Texture::from_poly_haven: files - JSON parse or download error, asset {}",
          asset_id);
      return false;
    }

    if (j.contains("Diffuse"))
    {
      for (auto &[key, value] : j["Diffuse"].items())
        this->diffuse_urls[key] = value["png"]["url"].get<std::string>();
    }

    if (j.contains("nor_gl"))
    {
      for (auto &[key, value] : j["nor_gl"].items())
        this->normal_urls[key] = value["png"]["url"].get<std::string>();
    }

    if (j.contains("Displacement"))
    {
      for (auto &[key, value] : j["Displacement"].items())
        this->displacement_urls[key] = value["png"]["url"].get<std::string>();
    }
  }

  return ret;
}

std::string Texture::get_id() const { return this->id; }

std::string Texture::get_name() const { return this->name; }

std::string Texture::get_source() const { return this->source; }

std::vector<TextureRes> Texture::get_texture_resolutions(const TextureType &texture_type)
{
  std::vector<TextureRes>            out;
  std::map<std::string, std::string> map;

  switch (texture_type)
  {
  case TextureType::DIFFUSE:
    map = this->diffuse_urls;
    break;
  case TextureType::NORMAL:
    map = this->normal_urls;
    break;
  case TextureType::DISPLACEMENT:
    map = this->displacement_urls;
    break;
  }

  for (auto &[key, _] : map)
  {
    // convert text to resolution enum
    TextureRes res = helper_poly_haven_res_converter(key);
    if (res != TextureRes::RUNKNOWN)
      out.push_back(res);
  }

  return out;
}

std::string Texture::get_texture_url(const TextureType &texture_type,
                                     const TextureRes  &texture_res)
{
  std::map<std::string, std::string> map;
  std::string                        res_key;

  switch (texture_type)
  {
  case TextureType::DIFFUSE:
    map = this->diffuse_urls;
    break;
  case TextureType::NORMAL:
    map = this->normal_urls;
    break;
  case TextureType::DISPLACEMENT:
    map = this->displacement_urls;
    break;
  }

  switch (texture_res)
  {
  case TextureRes::R1K:
    res_key = "1k";
    break;
  case TextureRes::R2K:
    res_key = "2k";
    break;
  case TextureRes::R4K:
    res_key = "4k";
    break;
  case TextureRes::R8K:
    res_key = "8k";
    break;
  }

  return map.at(res_key);
}

bool Texture::has_texture(const TextureType &texture_type)
{
  switch (texture_type)
  {
  case TextureType::DIFFUSE:
    return !this->diffuse_urls.empty();
    break;
  case TextureType::NORMAL:
    return !this->normal_urls.empty();
    break;
  case TextureType::DISPLACEMENT:
    return !this->displacement_urls.empty();
    break;
  }

  return false;
}

bool Texture::has_texture(const TextureType &texture_type, const TextureRes &texture_res)
{
  // type first
  if (!this->has_texture(texture_type))
    return false;

  // then check the resolution
  std::vector<TextureRes> available_res = this->get_texture_resolutions(texture_type);
  return contains(available_res, texture_res);
}

const QImage &Texture::get_thumbnail() const { return this->thumbnail; }

void Texture::json_from(nlohmann::json const &j)
{
  json_safe_get(j, "id", id);
  json_safe_get(j, "name", name);
  json_safe_get(j, "source", source);
  json_safe_get(j, "id_from_source", id_from_source);
  json_safe_get(j, "thumbnail_url", thumbnail_url);
  json_safe_get(j, "tags", tags);
  json_safe_get(j, "max_resolution", max_resolution);

  if (j.contains("thumbnail") && j["thumbnail"].is_string())
    thumbnail = qimage_from_base64(j["thumbnail"].get<std::string>());

  json_safe_get(j, "diffuse_urls", diffuse_urls);
  json_safe_get(j, "normal_urls", normal_urls);
  json_safe_get(j, "displacement_urls", displacement_urls);
}

nlohmann::json Texture::json_to() const
{
  nlohmann::json json;

  json = {{"id", id},
          {"name", name},
          {"source", source},
          {"id_from_source", id_from_source},
          {"thumbnail_url", thumbnail_url},
          {"tags", tags},
          {"max_resolution", max_resolution},
          {"thumbnail", qimage_to_base64(thumbnail)},
          {"diffuse_urls", diffuse_urls},
          {"normal_urls", normal_urls},
          {"displacement_urls", displacement_urls}};

  return json;
}

void Texture::set_id(const std::string &new_id) { this->id = new_id; }

} // namespace qtd
