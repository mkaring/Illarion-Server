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


#ifndef _FIELD_HPP_
#define _FIELD_HPP_

#include <vector>
#include <sys/socket.h>

#include "constants.hpp"
#include "globals.hpp"
#include "Container.hpp"
#include "Item.hpp"

namespace map {

class Field {
private:
    static const uint16_t TRANSPARENT = 0;

    uint16_t tile = 0;
    uint16_t music = 0;
    uint8_t flags = 0;
    position here;
    position warptarget;
    std::vector<Item> items;
    bool persistent = false;

public:
    Container::CONTAINERMAP containers;

public:
    explicit Field(const position &here): here(here) {};
    Field(const Field &) = delete;
    Field &operator=(const Field &) = delete;
    Field(Field &&) = default;
    Field &operator=(Field &&) = default;

    void setTileId(uint16_t id);
    uint16_t getTileId() const;
    uint16_t getSecondaryTileId() const;
    uint16_t getTileCode() const;
    bool isTransparent() const;

    void setMusicId(uint16_t id);
    uint16_t getMusicId() const;

    bool isWalkable() const;
    bool moveToPossible() const;
    TYPE_OF_WALKINGCOST getMovementCost() const;
    bool hasSpecialItem() const;

    bool addItemOnStack(const Item &item);
    bool addItemOnStackIfWalkable(const Item &item);
    bool takeItemFromStack(Item &item);
    int increaseItemOnStack(int count, bool &erased);
    bool swapItemOnStack(TYPE_OF_ITEM_ID newId, uint16_t newQuality = 0);
    bool viewItemOnStack(Item &item) const;
    ScriptItem getStackItem(uint8_t pos) const;
    const std::vector<Item> &getItemStack() const;
    MAXCOUNTTYPE itemCount() const;

    bool addContainerOnStackIfWalkable(Item item, Container *container);
    bool addContainerOnStack(Item item, Container *container);

    void age();

    void setPlayer();
    void setNPC();
    void setMonster();
    void removePlayer();
    void removeNPC();
    void removeMonster();
    bool hasPlayer() const;
    bool hasNPC() const;
    bool hasMonster() const;
    void setChar();
    void removeChar();

    void setWarp(const position &pos);
    void removeWarp();
    void getWarp(position &pos) const;
    bool isWarp() const;

    std::vector<Item> getExportItems() const;
    void save(std::ofstream &mapStream, std::ofstream &itemStream,
            std::ofstream &warpStream, std::ofstream &containerStream) const;
    void load(std::ifstream &mapStream, std::ifstream &itemStream,
            std::ifstream &warpStream, std::ifstream &containerStream);

    const position &getPosition() const;

    void makePersistent();
    void removePersistence();
    bool isPersistent() const;

private:
    void updateFlags();
    inline void setBits(uint8_t);
    inline void unsetBits(uint8_t);
    inline bool anyBitSet(uint8_t) const;

    void insertIntoDatabase() const noexcept;
    void removeFromDatabase() const noexcept;
    void updateDatabaseField() const noexcept;
    void updateDatabaseItems() const noexcept;
    void updateDatabaseWarp() const noexcept;
};

}

#endif
