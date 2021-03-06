//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#include "Item.hpp"

#include "constants.hpp"
#include "data/Data.hpp"
#include "script/LuaItemScript.hpp"
#include "script/server.hpp"
#include "stream.hpp"

#include <algorithm>
#include <range/v3/all.hpp>
#include <sstream>

auto ItemLookAt::operator==(const ItemLookAt &rhs) const -> bool {
    bool equal = true;
    equal &= (name == rhs.name);
    equal &= (rareness == rhs.rareness);
    equal &= (description == rhs.description);
    equal &= (craftedBy == rhs.craftedBy);
    equal &= (weight == rhs.weight);
    equal &= (worth == rhs.worth);
    equal &= (qualityText == rhs.qualityText);
    equal &= (durabilityText == rhs.durabilityText);
    equal &= (durabilityValue == rhs.durabilityValue);
    equal &= (diamondLevel == rhs.diamondLevel);
    equal &= (emeraldLevel == rhs.emeraldLevel);
    equal &= (rubyLevel == rhs.rubyLevel);
    equal &= (sapphireLevel == rhs.sapphireLevel);
    equal &= (amethystLevel == rhs.amethystLevel);
    equal &= (obsidianLevel == rhs.obsidianLevel);
    equal &= (topazLevel == rhs.topazLevel);
    equal &= (bonus == rhs.bonus);

    return equal;
}

auto ScriptItem::operator==(const ScriptItem &rhs) const -> bool {
    bool equal = (static_cast<const Item &>(*this) == rhs);
    equal &= (type == rhs.type);
    equal &= (pos == rhs.pos);
    equal &= (itempos == rhs.itempos);
    equal &= (owner == rhs.owner);
    equal &= (inside == rhs.inside);

    return equal;
}

auto Item::operator==(const Item &rhs) const -> bool {
    bool equal = true;
    equal &= (id == rhs.id);
    equal &= (number == rhs.number);
    equal &= (wear == rhs.wear);
    equal &= (quality == rhs.quality);
    equal &= (datamap == rhs.datamap);

    return equal;
}

Item::Item(id_type id, number_type number, wear_type wear, quality_type quality, const script_data_exchangemap &datamap)
        : id(id), number(number), wear(wear), quality(quality), datamap(1) {
    setData(&datamap);
}

auto Item::increaseNumberBy(Item::number_type count) -> number_type {
    const auto &itemStruct = Data::items()[id];

    if (itemStruct.isValid()) {
        count += getNumber();

        if (count >= itemStruct.MaxStack) {
            setNumber(itemStruct.MaxStack);
            count -= itemStruct.MaxStack;
        } else {
            setNumber(count);
            count = 0;
        }
    }

    return count;
}

void Item::setMinQuality(const Item &item) {
    quality_type minQuality = (quality < item.quality) ? quality : item.quality;
    minQuality /= maximumDurability + 1;
    quality_type minDurability = (getDurability() < item.getDurability()) ? getDurability() : item.getDurability();
    quality = minQuality * (maximumDurability + 1) + minDurability;
}

void Item::setData(script_data_exchangemap const *datamap) {
    if (datamap == nullptr) {
        this->datamap.clear();
        return;
    }

    for (const auto &item : *datamap) {
        setData(item.first, item.second);
    }
}

auto Item::hasData(const script_data_exchangemap &datamap) const -> bool {
    if (datamap.empty() && !hasNoData()) {
        return false;
    }

    using namespace ranges;
    auto dataEqual = [this](const auto &dataKeyValue) { return getData(dataKeyValue.first) == dataKeyValue.second; };
    return all_of(datamap, dataEqual);
}

auto Item::hasNoData() const -> bool { return datamap.empty(); }

auto Item::getData(const std::string &key) const -> std::string {
    if (datamap.find(key) != datamap.end()) {
        return datamap.at(key);
    }
    return "";
}

void Item::setData(const std::string &key, const std::string &value) {
    if (value.length() > 0) {
        datamap[key] = value;
    } else {
        datamap.erase(key);
    }
}

void Item::setData(const std::string &key, int32_t value) {
    std::stringstream ss;
    ss << value;
    setData(key, ss.str());
}

auto Item::getDepot() const -> uint16_t {
    uint16_t depot = 1;
    stringToNumber(getData("depot"), depot);
    return depot;
}

void Item::reset() {
    id = 0;
    number = 0;
    wear = 0;
    quality = defaultQuality;
    datamap.clear();
}

void Item::resetWear() {
    const auto &itemStruct = Data::items()[id];

    if (itemStruct.isValid()) {
        if (!itemStruct.rotsInInventory && itemStruct.AgeingSpeed > wear) {
            wear = itemStruct.AgeingSpeed;
        }
    }
}

void Item::save(std::ofstream &obj) const {
    writeToStream(obj, id);
    writeToStream(obj, number);
    writeToStream(obj, wear);
    writeToStream(obj, quality);
    const auto mapsize = static_cast<uint8_t>(datamap.size());
    writeToStream(obj, mapsize);

    for (const auto &data : datamap) {
        const auto sz1 = static_cast<uint8_t>(data.first.size());
        const auto sz2 = static_cast<uint8_t>(data.second.size());
        writeToStream(obj, sz1);
        writeToStream(obj, sz2);
        writeToStream(obj, data.first.data(), sz1);
        writeToStream(obj, data.second.data(), sz2);
    }
}

void Item::load(std::ifstream &obj) {
    readFromStream(obj, id);
    readFromStream(obj, number);
    readFromStream(obj, wear);
    readFromStream(obj, quality);
    uint8_t tempsize = 0;
    readFromStream(obj, tempsize);

    for (int i = 0; i < tempsize; ++i) {
        uint8_t sz1 = 0;
        uint8_t sz2 = 0;
        readFromStream(obj, sz1);
        readFromStream(obj, sz2);
        std::string key(sz1, '\0');
        readFromStream(obj, key.data(), sz1);
        std::string value(sz2, '\0');
        readFromStream(obj, value.data(), sz2);
        datamap[key] = value;
    }
}

auto Item::survivesAgeing() -> bool {
    if (wear != PERMANENT_WEAR && wear != 0) {
        --wear;
    }

    return wear > 0;
}

auto Item::isContainer() const -> bool { return Data::containerItems().exists(id); }

auto Item::getVolume() const -> TYPE_OF_VOLUME {
    const auto &itemStruct = Data::items()[id];

    if (itemStruct.isValid()) {
        return itemStruct.Volume;
    }

    return 0;
}

auto Item::getWeight() const -> TYPE_OF_WEIGHT {
    const auto &itemStruct = Data::items()[id];

    if (itemStruct.isValid()) {
        return itemStruct.Weight * number;
    }

    return 0;
}

auto Item::getWorth() const -> TYPE_OF_WORTH {
    const auto &itemStruct = Data::items()[id];

    if (itemStruct.isValid()) {
        return itemStruct.Worth * number;
    }

    return 0;
}

auto Item::getMaxStack() const -> number_type {
    const auto &itemStruct = Data::items()[id];

    if (itemStruct.isValid()) {
        return itemStruct.MaxStack;
    }

    return 0;
}

auto Item::isLarge() const -> bool { return getVolume() >= LARGE_ITEM_VOLUME; }

auto Item::isStackable() const -> bool { return getMaxStack() > 1; }

auto Item::isPermanent() const -> bool { return wear == PERMANENT_WEAR; }

auto Item::isMovable() const -> bool {
    const auto &itemStruct = Data::items()[id];

    if (itemStruct.isValid()) {
        return itemStruct.Weight < MAXWEIGHT && !isPermanent();
    }

    return false;
}

void Item::makePermanent() { wear = PERMANENT_WEAR; }

auto ScriptItem::getLookAt(Character *character) const -> ItemLookAt {
    auto script = Data::items().script(getId());

    if (script && script->existsEntrypoint("LookAtItem")) {
        ItemLookAt lookAt = script->LookAtItem(character, *this);

        if (lookAt.isValid()) {
            return lookAt;
        }
    }

    return script::server::lookAtItem().lookAtItem(character, *this);
}

auto ScriptItem::cloneItem() const -> Item {
    Item item{getId(), getNumber(), getWear(), getQuality()};
    std::for_each(getDataBegin(), getDataEnd(),
                  [&item](const auto &keyValue) { item.setData(keyValue.first, keyValue.second); });
    return item;
}
