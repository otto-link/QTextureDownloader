/* Copyright (c) 2025 Otto Link. Distributed under the terms of the GNU General Public
   License. The full license is in the file LICENSE, distributed with this software. */
#include <QGridLayout>
#include <QHeaderView>

#include "qtd/config.hpp"
#include "qtd/delegates.hpp"
#include "qtd/logger.hpp"
#include "qtd/texture_downloader.hpp"
#include "qtd/utils.hpp"

namespace qtd
{

TextureDownloader::TextureDownloader(const std::string &_title, QWidget *parent)
    : QWidget(parent), title(_title)
{
  QTD_LOG->trace("TextureDownloader::TextureDownloader");

  this->setWindowTitle(this->title.c_str());
  this->setFocusPolicy(Qt::StrongFocus);
  this->setMouseTracking(true);
  this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  this->setup_layout();
  this->setup_connections();

  this->texture_manager.load();
  this->update_table_rows();

  this->texture_manager.get_texture_rgba_16bit("PolyHaven_aerial_asphalt_01",
                                               TextureType::NORMAL,
                                               TextureRes::R1K);
}

void TextureDownloader::set_texture_res(const TextureRes &new_res)
{
  QTD_LOG->trace("TextureDownloader::set_texture_res");

  if (new_res == TextureRes::RUNKNOWN || new_res == this->res)
    return;

  this->res = new_res;
  this->update_table_rows();
}

void TextureDownloader::setup_connections()
{
  QTD_LOG->trace("TextureDownloader::setup_connections");

  this->connect(this->button_update,
                &QPushButton::clicked,
                this,
                &TextureDownloader::update_sources);

  this->connect(this->button_uncheck_items,
                &QPushButton::clicked,
                this,
                &TextureDownloader::unchecked_all_items);

  this->connect(this->combo_res,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                [this]()
                {
                  std::string res_name = this->combo_res->currentText().toStdString();
                  TextureRes  new_res;
                  for (auto &[r, str] : texture_res_as_string)
                  {
                    if (str == res_name)
                    {
                      new_res = r;
                      break;
                    }
                  }
                  this->set_texture_res(new_res);
                });
}

void TextureDownloader::setup_layout()
{
  QTD_LOG->trace("TextureDownloader::setup_layout");

  // create and assign new layout
  QGridLayout *layout = new QGridLayout(this);
  layout->setContentsMargins(2, 0, 2, 0);
  this->setLayout(layout);

  /// --- first line

  // combo
  this->combo_res = new QComboBox();
  layout->addWidget(this->combo_res, 0, 0);

  for (auto &r : all_texture_res)
  {
    std::string res_name = texture_res_as_string.at(r);
    this->combo_res->addItem(res_name.c_str());
  }

  std::string current_res_str = texture_res_as_string.at(this->res);
  int         idx = this->combo_res->findText(current_res_str.c_str());
  if (idx >= 0)
    this->combo_res->setCurrentIndex(idx);

  // clear checked items
  this->button_uncheck_items = new QPushButton("Uncheck items");
  layout->addWidget(this->button_uncheck_items, 0, 1);

  // update all
  this->button_update = new QPushButton("Update sources");
  layout->addWidget(this->button_update, 0, 2);

  // --- table

  this->table_model = new QStandardItemModel(0, 3, this);

  // labels
  QStringList labels = {"Thumbnail", "ID", "Name", "Source"};
  for (auto &r : all_texture_types)
  {
    std::string type_name = texture_type_as_string.at(r);
    labels << type_name.c_str();
  }

  this->table_model->setHorizontalHeaderLabels(labels);

  this->table_view = new QTableView(this);
  this->table_view->setModel(this->table_model);
  this->table_view->setSortingEnabled(true);
  this->table_view->horizontalHeader()->setStretchLastSection(true);
  this->table_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
  this->table_view->verticalHeader()->setDefaultSectionSize(
      QTD_CONFIG->widget.thumbnail_size.height());
  this->table_view->setItemDelegateForColumn(0, new ThumbnailDelegate(this->table_view));

  layout->addWidget(this->table_view, 1, 0, 1, 3);
}

QSize TextureDownloader::sizeHint() const { return QSize(QTD_CONFIG->widget.size_hint); }

void TextureDownloader::unchecked_all_items()
{
  for (int row = 0; row < this->table_model->rowCount(); ++row)
    for (int col = 0; col < this->table_model->columnCount(); ++col)
    {
      QStandardItem *item = this->table_model->item(row, col);
      if (item && item->isCheckable())
      {
        item->setCheckState(Qt::Unchecked);
      }
    }
}

void TextureDownloader::update_sources()
{
  QTD_LOG->trace("TextureDownloader::update_sources");

  // TODO add dialog box "sure?"

  this->texture_manager.update();
  this->texture_manager.save();
  this->update_table_rows();
}

void TextureDownloader::update_table_rows()
{
  QTD_LOG->trace("TextureDownloader::update_table_rows");

  this->table_model->removeRows(0, this->table_model->rowCount());

  for (auto &[id, tex] : this->texture_manager.get_textures())
  {

    // thumbnail
    QPixmap        pix = QPixmap::fromImage(tex.get_thumbnail());
    QStandardItem *img_item = new QStandardItem;
    img_item->setData(pix, Qt::DecorationRole);

    QList<QStandardItem *> items = {img_item,
                                    new QStandardItem(id.c_str()),
                                    new QStandardItem(tex.get_name().c_str()),
                                    new QStandardItem(tex.get_source().c_str())};

    // add textures
    for (auto &r : all_texture_types)
    {
      if (tex.has_texture(r, this->res))
      {
        QStandardItem *check_item = new QStandardItem;
        check_item->setCheckable(true);
        check_item->setCheckState(Qt::Unchecked);
        items.append(check_item);
      }
      else
      {
        items.append(new QStandardItem("Not available"));
      }
    }

    this->table_model->appendRow(items);
  }
}

} // namespace qtd
