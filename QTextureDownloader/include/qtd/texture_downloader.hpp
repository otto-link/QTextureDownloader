/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General Public
   License. The full license is in the file LICENSE, distributed with this software. */
#pragma once
#include <string>

#include <QComboBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>
#include <QWidget>

#include "nlohmann/json.hpp"

#include "qtd/texture_manager.hpp"

namespace qtd
{

class TextureDownloader : public QWidget
{
  Q_OBJECT

public:
  explicit TextureDownloader(const std::string &_title = "", QWidget *parent = nullptr);
  ~TextureDownloader();

  // --- Serialization
  //   void           json_from(nlohmann::json const &json);
  //   nlohmann::json json_to() const;

  void set_texture_res(const TextureRes &new_res);

  // --- QWidget interface
  QSize sizeHint() const override;

signals:
  void textures_retrieved(const std::vector<std::string> &texture_paths);

public slots:
  void retrieve_selected_textures();
  void unchecked_all_items();
  void update_sources();

private:
  void setup_connections();
  void setup_layout();
  void update_table_rows();

  // --- Members
  std::string    title;
  TextureManager texture_manager;
  TextureRes     res = TextureRes::R1K;

  QPushButton        *button_get_selected;
  QPushButton        *button_update;
  QPushButton        *button_uncheck_items;
  QComboBox          *combo_res;
  QStandardItemModel *table_model;
  QTableView         *table_view;
};

} // namespace qtd