#include "refidadapterimp.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

#include <components/esm/loadcont.hpp>
#include <components/esm/attr.hpp>

#include "nestedtablewrapper.hpp"
#include "usertype.hpp"
#include "idtree.hpp"
#include "npcstats.hpp"

CSMWorld::PotionColumns::PotionColumns (const InventoryColumns& columns)
: InventoryColumns (columns) {}

CSMWorld::PotionRefIdAdapter::PotionRefIdAdapter (const PotionColumns& columns,
    const RefIdColumn *autoCalc)
: InventoryRefIdAdapter<ESM::Potion> (UniversalId::Type_Potion, columns),
  mColumns(columns), mAutoCalc (autoCalc)
{}

QVariant CSMWorld::PotionRefIdAdapter::getData (const RefIdColumn *column, const RefIdData& data,
    int index) const
{
    const Record<ESM::Potion>& record = static_cast<const Record<ESM::Potion>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Potion)));

    if (column==mAutoCalc)
        return record.get().mData.mAutoCalc!=0;

    if (column==mColumns.mEffects)
        return true; // to show nested tables in dialogue subview, see IdTree::hasChildren()

    return InventoryRefIdAdapter<ESM::Potion>::getData (column, data, index);
}

void CSMWorld::PotionRefIdAdapter::setData (const RefIdColumn *column, RefIdData& data, int index,
    const QVariant& value) const
{
    Record<ESM::Potion>& record = static_cast<Record<ESM::Potion>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Potion)));

    ESM::Potion potion = record.get();

    if (column==mAutoCalc)
        potion.mData.mAutoCalc = value.toInt();
    else
    {
        InventoryRefIdAdapter<ESM::Potion>::setData (column, data, index, value);

        return;
    }

    record.setModified(potion);
}


CSMWorld::IngredientColumns::IngredientColumns (const InventoryColumns& columns)
: InventoryColumns (columns) {}

CSMWorld::IngredientRefIdAdapter::IngredientRefIdAdapter (const IngredientColumns& columns)
: InventoryRefIdAdapter<ESM::Ingredient> (UniversalId::Type_Ingredient, columns),
  mColumns(columns)
{}

QVariant CSMWorld::IngredientRefIdAdapter::getData (const RefIdColumn *column, const RefIdData& data,
    int index) const
{
    const Record<ESM::Ingredient>& record = static_cast<const Record<ESM::Ingredient>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Ingredient)));

    if (column==mColumns.mEffects)
        return true; // to show nested tables in dialogue subview, see IdTree::hasChildren()

    return InventoryRefIdAdapter<ESM::Ingredient>::getData (column, data, index);
}

void CSMWorld::IngredientRefIdAdapter::setData (const RefIdColumn *column, RefIdData& data, int index,
    const QVariant& value) const
{
    InventoryRefIdAdapter<ESM::Ingredient>::setData (column, data, index, value);

    return;
}


CSMWorld::IngredEffectRefIdAdapter::IngredEffectRefIdAdapter()
: mType(UniversalId::Type_Ingredient)
{}

CSMWorld::IngredEffectRefIdAdapter::~IngredEffectRefIdAdapter()
{}

void CSMWorld::IngredEffectRefIdAdapter::addNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int position) const
{
    // Do nothing, this table cannot be changed by the user
}

void CSMWorld::IngredEffectRefIdAdapter::removeNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int rowToRemove) const
{
    // Do nothing, this table cannot be changed by the user
}

void CSMWorld::IngredEffectRefIdAdapter::setNestedTable (const RefIdColumn* column,
        RefIdData& data, int index, const NestedTableWrapperBase& nestedTable) const
{
    Record<ESM::Ingredient>& record =
        static_cast<Record<ESM::Ingredient>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));
    ESM::Ingredient ingredient = record.get();

    ingredient.mData =
        static_cast<const NestedTableWrapper<std::vector<typename ESM::Ingredient::IRDTstruct> >&>(nestedTable).mNestedTable.at(0);

    record.setModified (ingredient);
}

CSMWorld::NestedTableWrapperBase* CSMWorld::IngredEffectRefIdAdapter::nestedTable (const RefIdColumn* column,
        const RefIdData& data, int index) const
{
    const Record<ESM::Ingredient>& record =
        static_cast<const Record<ESM::Ingredient>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));

    // return the whole struct
    std::vector<typename ESM::Ingredient::IRDTstruct> wrap;
    wrap.push_back(record.get().mData);

    // deleted by dtor of NestedTableStoring
    return new NestedTableWrapper<std::vector<typename ESM::Ingredient::IRDTstruct> >(wrap);
}

QVariant CSMWorld::IngredEffectRefIdAdapter::getNestedData (const RefIdColumn *column,
        const RefIdData& data, int index, int subRowIndex, int subColIndex) const
{
    const Record<ESM::Ingredient>& record =
        static_cast<const Record<ESM::Ingredient>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));

    if (subRowIndex < 0 || subRowIndex >= 4)
        throw std::runtime_error ("index out of range");

    switch (subColIndex)
    {
        case 0: return record.get().mData.mEffectID[subRowIndex];
        case 1: return record.get().mData.mSkills[subRowIndex];
        case 2: return record.get().mData.mAttributes[subRowIndex];
        default:
            throw std::runtime_error("Trying to access non-existing column in the nested table!");
    }
}

void CSMWorld::IngredEffectRefIdAdapter::setNestedData (const RefIdColumn *column,
        RefIdData& data, int row, const QVariant& value, int subRowIndex, int subColIndex) const
{
    Record<ESM::Ingredient>& record =
        static_cast<Record<ESM::Ingredient>&> (data.getRecord (RefIdData::LocalIndex (row, mType)));
    ESM::Ingredient ingredient = record.get();

    if (subRowIndex < 0 || subRowIndex >= 4)
        throw std::runtime_error ("index out of range");

    switch(subColIndex)
    {
        case 0: ingredient.mData.mEffectID[subRowIndex] = value.toInt(); break;
        case 1: ingredient.mData.mSkills[subRowIndex] = value.toInt(); break;
        case 2: ingredient.mData.mAttributes[subRowIndex] = value.toInt(); break;
        default:
            throw std::runtime_error("Trying to access non-existing column in the nested table!");
    }

    record.setModified (ingredient);
}

int CSMWorld::IngredEffectRefIdAdapter::getNestedColumnsCount(const RefIdColumn *column, const RefIdData& data) const
{
    return 3; // effect, skill, attribute
}

int CSMWorld::IngredEffectRefIdAdapter::getNestedRowsCount(const RefIdColumn *column, const RefIdData& data, int index) const
{
    return 4; // up to 4 effects
}


CSMWorld::ApparatusRefIdAdapter::ApparatusRefIdAdapter (const InventoryColumns& columns,
    const RefIdColumn *type, const RefIdColumn *quality)
: InventoryRefIdAdapter<ESM::Apparatus> (UniversalId::Type_Apparatus, columns),
    mType (type), mQuality (quality)
{}

QVariant CSMWorld::ApparatusRefIdAdapter::getData (const RefIdColumn *column,
    const RefIdData& data, int index) const
{
    const Record<ESM::Apparatus>& record = static_cast<const Record<ESM::Apparatus>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Apparatus)));

    if (column==mType)
        return record.get().mData.mType;

    if (column==mQuality)
        return record.get().mData.mQuality;

    return InventoryRefIdAdapter<ESM::Apparatus>::getData (column, data, index);
}

void CSMWorld::ApparatusRefIdAdapter::setData (const RefIdColumn *column, RefIdData& data, int index,
    const QVariant& value) const
{
    Record<ESM::Apparatus>& record = static_cast<Record<ESM::Apparatus>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Apparatus)));

    ESM::Apparatus apparatus = record.get();

    if (column==mType)
        apparatus.mData.mType = value.toInt();
    else if (column==mQuality)
        apparatus.mData.mQuality = value.toFloat();
    else
    {
        InventoryRefIdAdapter<ESM::Apparatus>::setData (column, data, index, value);

        return;
    }
    record.setModified(apparatus);
}


CSMWorld::ArmorRefIdAdapter::ArmorRefIdAdapter (const EnchantableColumns& columns,
    const RefIdColumn *type, const RefIdColumn *health, const RefIdColumn *armor,
    const RefIdColumn *partRef)
: EnchantableRefIdAdapter<ESM::Armor> (UniversalId::Type_Armor, columns),
    mType (type), mHealth (health), mArmor (armor), mPartRef(partRef)
{}

QVariant CSMWorld::ArmorRefIdAdapter::getData (const RefIdColumn *column,
    const RefIdData& data, int index) const
{
    const Record<ESM::Armor>& record = static_cast<const Record<ESM::Armor>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Armor)));

    if (column==mType)
        return record.get().mData.mType;

    if (column==mHealth)
        return record.get().mData.mHealth;

    if (column==mArmor)
        return record.get().mData.mArmor;

    if (column==mPartRef)
        return true; // to show nested tables in dialogue subview, see IdTree::hasChildren()

    return EnchantableRefIdAdapter<ESM::Armor>::getData (column, data, index);
}

void CSMWorld::ArmorRefIdAdapter::setData (const RefIdColumn *column, RefIdData& data, int index,
    const QVariant& value) const
{
    Record<ESM::Armor>& record = static_cast<Record<ESM::Armor>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Armor)));

    ESM::Armor armor = record.get();

    if (column==mType)
        armor.mData.mType = value.toInt();
    else if (column==mHealth)
        armor.mData.mHealth = value.toInt();
    else if (column==mArmor)
        armor.mData.mArmor = value.toInt();
    else
    {
        EnchantableRefIdAdapter<ESM::Armor>::setData (column, data, index, value);

        return;
    }

    record.setModified(armor);
}

CSMWorld::BookRefIdAdapter::BookRefIdAdapter (const EnchantableColumns& columns,
    const RefIdColumn *scroll, const RefIdColumn *skill)
: EnchantableRefIdAdapter<ESM::Book> (UniversalId::Type_Book, columns),
    mScroll (scroll), mSkill (skill)
{}

QVariant CSMWorld::BookRefIdAdapter::getData (const RefIdColumn *column,
    const RefIdData& data, int index) const
{
    const Record<ESM::Book>& record = static_cast<const Record<ESM::Book>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Book)));

    if (column==mScroll)
        return record.get().mData.mIsScroll!=0;

    if (column==mSkill)
        return record.get().mData.mSkillID;

    return EnchantableRefIdAdapter<ESM::Book>::getData (column, data, index);
}

void CSMWorld::BookRefIdAdapter::setData (const RefIdColumn *column, RefIdData& data, int index,
    const QVariant& value) const
{
    Record<ESM::Book>& record = static_cast<Record<ESM::Book>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Book)));

    ESM::Book book = record.get();

    if (column==mScroll)
        book.mData.mIsScroll = value.toInt();
    else if (column==mSkill)
        book.mData.mSkillID = value.toInt();
    else
    {
        EnchantableRefIdAdapter<ESM::Book>::setData (column, data, index, value);

        return;
    }

    record.setModified(book);
}

CSMWorld::ClothingRefIdAdapter::ClothingRefIdAdapter (const EnchantableColumns& columns,
    const RefIdColumn *type, const RefIdColumn *partRef)
: EnchantableRefIdAdapter<ESM::Clothing> (UniversalId::Type_Clothing, columns), mType (type),
  mPartRef(partRef)
{}

QVariant CSMWorld::ClothingRefIdAdapter::getData (const RefIdColumn *column,
    const RefIdData& data, int index) const
{
    const Record<ESM::Clothing>& record = static_cast<const Record<ESM::Clothing>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Clothing)));

    if (column==mType)
        return record.get().mData.mType;

    if (column==mPartRef)
        return true; // to show nested tables in dialogue subview, see IdTree::hasChildren()

    return EnchantableRefIdAdapter<ESM::Clothing>::getData (column, data, index);
}

void CSMWorld::ClothingRefIdAdapter::setData (const RefIdColumn *column, RefIdData& data, int index,
    const QVariant& value) const
{
    Record<ESM::Clothing>& record = static_cast<Record<ESM::Clothing>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Clothing)));

    ESM::Clothing clothing = record.get();

    if (column==mType)
        clothing.mData.mType = value.toInt();
    else
    {
        EnchantableRefIdAdapter<ESM::Clothing>::setData (column, data, index, value);

        return;
    }

    record.setModified(clothing);
}

CSMWorld::ContainerRefIdAdapter::ContainerRefIdAdapter (const NameColumns& columns,
    const RefIdColumn *weight, const RefIdColumn *organic, const RefIdColumn *respawn, const RefIdColumn *content)
: NameRefIdAdapter<ESM::Container> (UniversalId::Type_Container, columns), mWeight (weight),
  mOrganic (organic), mRespawn (respawn), mContent(content)
{}

QVariant CSMWorld::ContainerRefIdAdapter::getData (const RefIdColumn *column,
                                                   const RefIdData& data,
                                                   int index) const
{
    const Record<ESM::Container>& record = static_cast<const Record<ESM::Container>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Container)));

    if (column==mWeight)
        return record.get().mWeight;

    if (column==mOrganic)
        return (record.get().mFlags & ESM::Container::Organic)!=0;

    if (column==mRespawn)
        return (record.get().mFlags & ESM::Container::Respawn)!=0;

    if (column==mContent)
        return true; // Required to show nested tables in dialogue subview

    return NameRefIdAdapter<ESM::Container>::getData (column, data, index);
}

void CSMWorld::ContainerRefIdAdapter::setData (const RefIdColumn *column, RefIdData& data, int index,
    const QVariant& value) const
{
    Record<ESM::Container>& record = static_cast<Record<ESM::Container>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Container)));

    ESM::Container container = record.get();

    if (column==mWeight)
        container.mWeight = value.toFloat();
    else if (column==mOrganic)
    {
        if (value.toInt())
            container.mFlags |= ESM::Container::Organic;
        else
            container.mFlags &= ~ESM::Container::Organic;
    }
    else if (column==mRespawn)
    {
        if (value.toInt())
            container.mFlags |= ESM::Container::Respawn;
        else
            container.mFlags &= ~ESM::Container::Respawn;
    }
    else
    {
        NameRefIdAdapter<ESM::Container>::setData (column, data, index, value);

        return;
    }

    record.setModified(container);
}

CSMWorld::CreatureColumns::CreatureColumns (const ActorColumns& actorColumns)
: ActorColumns (actorColumns),
  mType(NULL),
  mScale(NULL),
  mOriginal(NULL),
  mAttributes(NULL),
  mAttacks(NULL),
  mMisc(NULL)
{}

CSMWorld::CreatureRefIdAdapter::CreatureRefIdAdapter (const CreatureColumns& columns)
: ActorRefIdAdapter<ESM::Creature> (UniversalId::Type_Creature, columns), mColumns (columns)
{}

QVariant CSMWorld::CreatureRefIdAdapter::getData (const RefIdColumn *column, const RefIdData& data,
    int index) const
{
    const Record<ESM::Creature>& record = static_cast<const Record<ESM::Creature>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Creature)));

    if (column==mColumns.mType)
        return record.get().mData.mType;

    if (column==mColumns.mScale)
        return record.get().mScale;

    if (column==mColumns.mOriginal)
        return QString::fromUtf8 (record.get().mOriginal.c_str());

    if (column==mColumns.mAttributes)
        return true; // Required to show nested tables in dialogue subview

    if (column==mColumns.mAttacks)
        return true; // Required to show nested tables in dialogue subview

    if (column==mColumns.mMisc)
        return true; // Required to show nested items in dialogue subview

    std::map<const RefIdColumn *, unsigned int>::const_iterator iter =
        mColumns.mFlags.find (column);

    if (iter!=mColumns.mFlags.end())
        return (record.get().mFlags & iter->second)!=0;

    return ActorRefIdAdapter<ESM::Creature>::getData (column, data, index);
}

void CSMWorld::CreatureRefIdAdapter::setData (const RefIdColumn *column, RefIdData& data, int index,
    const QVariant& value) const
{
    Record<ESM::Creature>& record = static_cast<Record<ESM::Creature>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Creature)));

    ESM::Creature creature = record.get();

    if (column==mColumns.mType)
        creature.mData.mType = value.toInt();
    else if (column==mColumns.mScale)
        creature.mScale = value.toFloat();
    else if (column==mColumns.mOriginal)
        creature.mOriginal = value.toString().toUtf8().constData();
    else
    {
        std::map<const RefIdColumn *, unsigned int>::const_iterator iter =
            mColumns.mFlags.find (column);

        if (iter!=mColumns.mFlags.end())
        {
            if (value.toInt()!=0)
                creature.mFlags |= iter->second;
            else
                creature.mFlags &= ~iter->second;
        }
        else
        {
            ActorRefIdAdapter<ESM::Creature>::setData (column, data, index, value);

            return;
        }
    }

    record.setModified(creature);
}

CSMWorld::DoorRefIdAdapter::DoorRefIdAdapter (const NameColumns& columns,
    const RefIdColumn *openSound, const RefIdColumn *closeSound)
: NameRefIdAdapter<ESM::Door> (UniversalId::Type_Door, columns), mOpenSound (openSound),
  mCloseSound (closeSound)
{}

QVariant CSMWorld::DoorRefIdAdapter::getData (const RefIdColumn *column, const RefIdData& data,
    int index) const
{
    const Record<ESM::Door>& record = static_cast<const Record<ESM::Door>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Door)));

    if (column==mOpenSound)
        return QString::fromUtf8 (record.get().mOpenSound.c_str());

    if (column==mCloseSound)
        return QString::fromUtf8 (record.get().mCloseSound.c_str());

    return NameRefIdAdapter<ESM::Door>::getData (column, data, index);
}

void CSMWorld::DoorRefIdAdapter::setData (const RefIdColumn *column, RefIdData& data, int index,
    const QVariant& value) const
{
    Record<ESM::Door>& record = static_cast<Record<ESM::Door>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Door)));

    ESM::Door door = record.get();

    if (column==mOpenSound)
        door.mOpenSound = value.toString().toUtf8().constData();
    else if (column==mCloseSound)
        door.mCloseSound = value.toString().toUtf8().constData();
    else
    {
        NameRefIdAdapter<ESM::Door>::setData (column, data, index, value);

        return;
    }

    record.setModified(door);
}

CSMWorld::LightColumns::LightColumns (const InventoryColumns& columns)
: InventoryColumns (columns) {}

CSMWorld::LightRefIdAdapter::LightRefIdAdapter (const LightColumns& columns)
: InventoryRefIdAdapter<ESM::Light> (UniversalId::Type_Light, columns), mColumns (columns)
{}

QVariant CSMWorld::LightRefIdAdapter::getData (const RefIdColumn *column, const RefIdData& data,
    int index) const
{
    const Record<ESM::Light>& record = static_cast<const Record<ESM::Light>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Light)));

    if (column==mColumns.mTime)
        return record.get().mData.mTime;

    if (column==mColumns.mRadius)
        return record.get().mData.mRadius;

    if (column==mColumns.mColor)
        return record.get().mData.mColor;

    if (column==mColumns.mSound)
        return QString::fromUtf8 (record.get().mSound.c_str());

    std::map<const RefIdColumn *, unsigned int>::const_iterator iter =
        mColumns.mFlags.find (column);

    if (iter!=mColumns.mFlags.end())
        return (record.get().mData.mFlags & iter->second)!=0;

    return InventoryRefIdAdapter<ESM::Light>::getData (column, data, index);
}

void CSMWorld::LightRefIdAdapter::setData (const RefIdColumn *column, RefIdData& data, int index,
    const QVariant& value) const
{
    Record<ESM::Light>& record = static_cast<Record<ESM::Light>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Light)));

    ESM::Light light = record.get();

    if (column==mColumns.mTime)
        light.mData.mTime = value.toInt();
    else if (column==mColumns.mRadius)
        light.mData.mRadius = value.toInt();
    else if (column==mColumns.mColor)
        light.mData.mColor = value.toInt();
    else if (column==mColumns.mSound)
        light.mSound = value.toString().toUtf8().constData();
    else
    {
        std::map<const RefIdColumn *, unsigned int>::const_iterator iter =
            mColumns.mFlags.find (column);

        if (iter!=mColumns.mFlags.end())
        {
            if (value.toInt()!=0)
                light.mData.mFlags |= iter->second;
            else
                light.mData.mFlags &= ~iter->second;
        }
        else
        {
            InventoryRefIdAdapter<ESM::Light>::setData (column, data, index, value);

            return;
        }
    }

    record.setModified (light);
}

CSMWorld::MiscRefIdAdapter::MiscRefIdAdapter (const InventoryColumns& columns, const RefIdColumn *key)
: InventoryRefIdAdapter<ESM::Miscellaneous> (UniversalId::Type_Miscellaneous, columns), mKey (key)
{}

QVariant CSMWorld::MiscRefIdAdapter::getData (const RefIdColumn *column, const RefIdData& data,
    int index) const
{
    const Record<ESM::Miscellaneous>& record = static_cast<const Record<ESM::Miscellaneous>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Miscellaneous)));

    if (column==mKey)
        return record.get().mData.mIsKey!=0;

    return InventoryRefIdAdapter<ESM::Miscellaneous>::getData (column, data, index);
}

void CSMWorld::MiscRefIdAdapter::setData (const RefIdColumn *column, RefIdData& data, int index,
    const QVariant& value) const
{
    Record<ESM::Miscellaneous>& record = static_cast<Record<ESM::Miscellaneous>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Miscellaneous)));

    ESM::Miscellaneous misc = record.get();

    if (column==mKey)
        misc.mData.mIsKey = value.toInt();
    else
    {
        InventoryRefIdAdapter<ESM::Miscellaneous>::setData (column, data, index, value);

        return;
    }

    record.setModified(misc);
}

CSMWorld::NpcColumns::NpcColumns (const ActorColumns& actorColumns)
: ActorColumns (actorColumns),
  mRace(NULL),
  mClass(NULL),
  mFaction(NULL),
  mHair(NULL),
  mHead(NULL),
  mAttributes(NULL),
  mSkills(NULL),
  mMisc(NULL)
{}

CSMWorld::NpcRefIdAdapter::NpcRefIdAdapter (const NpcColumns& columns, const CSMWorld::Data& data)
: ActorRefIdAdapter<ESM::NPC> (UniversalId::Type_Npc, columns), mColumns (columns), mData(data)
{}

QVariant CSMWorld::NpcRefIdAdapter::getData (const RefIdColumn *column, const RefIdData& data, int index)
    const
{
    const Record<ESM::NPC>& record = static_cast<const Record<ESM::NPC>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Npc)));

    if (column==mColumns.mRace)
        return QString::fromUtf8 (record.get().mRace.c_str());

    if (column==mColumns.mClass)
        return QString::fromUtf8 (record.get().mClass.c_str());

    if (column==mColumns.mFaction)
        return QString::fromUtf8 (record.get().mFaction.c_str());

    if (column==mColumns.mHair)
        return QString::fromUtf8 (record.get().mHair.c_str());

    if (column==mColumns.mHead)
        return QString::fromUtf8 (record.get().mHead.c_str());

    if (column==mColumns.mAttributes || column==mColumns.mSkills)
    {
        if ((record.get().mFlags & ESM::NPC::Autocalc) != 0)
            return QVariant(QVariant::UserType);
        else
            return true;
    }

    if (column==mColumns.mMisc)
        return true;

    std::map<const RefIdColumn *, unsigned int>::const_iterator iter =
        mColumns.mFlags.find (column);

    if (iter!=mColumns.mFlags.end())
        return (record.get().mFlags & iter->second)!=0;

    return ActorRefIdAdapter<ESM::NPC>::getData (column, data, index);
}

void CSMWorld::NpcRefIdAdapter::setData (const RefIdColumn *column, RefIdData& data, int index,
    const QVariant& value) const
{
    Record<ESM::NPC>& record = static_cast<Record<ESM::NPC>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Npc)));

    ESM::NPC npc = record.get();

    if (column==mColumns.mRace)
        npc.mRace = value.toString().toUtf8().constData();
    else if (column==mColumns.mClass)
        npc.mClass = value.toString().toUtf8().constData();
    else if (column==mColumns.mFaction)
        npc.mFaction = value.toString().toUtf8().constData();
    else if (column==mColumns.mHair)
        npc.mHair = value.toString().toUtf8().constData();
    else if (column==mColumns.mHead)
        npc.mHead = value.toString().toUtf8().constData();
    else
    {
        std::map<const RefIdColumn *, unsigned int>::const_iterator iter =
            mColumns.mFlags.find (column);

        if (iter!=mColumns.mFlags.end())
        {
            if (value.toInt()!=0)
                npc.mFlags |= iter->second;
            else
                npc.mFlags &= ~iter->second;

            if (iter->second == ESM::NPC::Autocalc)
            {
                if (npc.mNpdtType == ESM::NPC::NPC_WITH_AUTOCALCULATED_STATS)
                {
                    CSMWorld::NpcStats *stats = mData.npcAutoCalculate(npc);
                    if (!stats)
                    {
                        record.setModified (npc);
                        return;
                    }

                    // update npc
                    npc.mNpdtType = ESM::NPC::NPC_DEFAULT;
                    npc.mNpdt52.mLevel        = npc.mNpdt12.mLevel;

                    npc.mNpdt52.mStrength     = stats->getBaseAttribute(ESM::Attribute::Strength);
                    npc.mNpdt52.mIntelligence = stats->getBaseAttribute(ESM::Attribute::Intelligence);
                    npc.mNpdt52.mWillpower    = stats->getBaseAttribute(ESM::Attribute::Willpower);
                    npc.mNpdt52.mAgility      = stats->getBaseAttribute(ESM::Attribute::Agility);
                    npc.mNpdt52.mSpeed        = stats->getBaseAttribute(ESM::Attribute::Speed);
                    npc.mNpdt52.mEndurance    = stats->getBaseAttribute(ESM::Attribute::Endurance);
                    npc.mNpdt52.mPersonality  = stats->getBaseAttribute(ESM::Attribute::Personality);
                    npc.mNpdt52.mLuck         = stats->getBaseAttribute(ESM::Attribute::Luck);

                    for (int i = 0; i < ESM::Skill::Length; ++i)
                    {
                        npc.mNpdt52.mSkills[i] = stats->getBaseSkill(i);
                    }

                    npc.mNpdt52.mHealth       = stats->getHealth();
                    npc.mNpdt52.mMana         = stats->getMana();
                    npc.mNpdt52.mFatigue      = stats->getFatigue();
                    npc.mNpdt52.mDisposition  = npc.mNpdt12.mDisposition;
                    npc.mNpdt52.mReputation   = npc.mNpdt12.mReputation;
                    npc.mNpdt52.mRank         = npc.mNpdt12.mRank;
                    npc.mNpdt52.mGold         = npc.mNpdt12.mGold;

                    // TODO: add spells from autogenerated list like vanilla (but excluding any
                    // race powers or abilities)
                }
                else
                {
                    npc.mNpdtType =  ESM::NPC::NPC_WITH_AUTOCALCULATED_STATS;
                    npc.mNpdt12.mLevel = npc.mNpdt52.mLevel; // for NPC's loaded as non-autocalc
                    mData.npcAutoCalculate(npc);
                }
            }
        }
        else
        {
            ActorRefIdAdapter<ESM::NPC>::setData (column, data, index, value);

            return;
        }
    }

    record.setModified (npc);
}

CSMWorld::NpcAttributesRefIdAdapter::NpcAttributesRefIdAdapter(const CSMWorld::Data& data) : mData(data)
{}

void CSMWorld::NpcAttributesRefIdAdapter::addNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int position) const
{
    // Do nothing, this table cannot be changed by the user
}

void CSMWorld::NpcAttributesRefIdAdapter::removeNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int rowToRemove) const
{
    // Do nothing, this table cannot be changed by the user
}

void CSMWorld::NpcAttributesRefIdAdapter::setNestedTable (const RefIdColumn* column,
        RefIdData& data, int index, const NestedTableWrapperBase& nestedTable) const
{
    Record<ESM::NPC>& record =
        static_cast<Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Npc)));
    ESM::NPC npc = record.get();

    // store the whole struct
    npc.mNpdt52 =
        static_cast<const NestedTableWrapper<std::vector<typename ESM::NPC::NPDTstruct52> > &>(nestedTable).mNestedTable.at(0);

    record.setModified (npc);
}

CSMWorld::NestedTableWrapperBase* CSMWorld::NpcAttributesRefIdAdapter::nestedTable (const RefIdColumn* column,
        const RefIdData& data, int index) const
{
    const Record<ESM::NPC>& record =
        static_cast<const Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Npc)));

    // return the whole struct
    std::vector<typename ESM::NPC::NPDTstruct52> wrap;
    wrap.push_back(record.get().mNpdt52);
    // deleted by dtor of NestedTableStoring
    return new NestedTableWrapper<std::vector<typename ESM::NPC::NPDTstruct52> >(wrap);
}

QVariant CSMWorld::NpcAttributesRefIdAdapter::getNestedData (const RefIdColumn *column,
        const RefIdData& data, int index, int subRowIndex, int subColIndex) const
{
    const Record<ESM::NPC>& record =
        static_cast<const Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Npc)));

    const ESM::NPC npc = record.get();
    const ESM::NPC::NPDTstruct52& npcStruct = npc.mNpdt52;

    if (subColIndex == 0)
        return subRowIndex;
    else if (subColIndex == 1)
        if (npc.mNpdtType == ESM::NPC::NPC_WITH_AUTOCALCULATED_STATS)
        {
            CSMWorld::NpcStats *stats =  mData.npcAutoCalculate(npc);
            if (!stats)
                return QVariant();

            switch (subRowIndex)
            {
                case 0: return static_cast<int>(stats->getBaseAttribute(ESM::Attribute::Strength));
                case 1: return static_cast<int>(stats->getBaseAttribute(ESM::Attribute::Intelligence));
                case 2: return static_cast<int>(stats->getBaseAttribute(ESM::Attribute::Willpower));
                case 3: return static_cast<int>(stats->getBaseAttribute(ESM::Attribute::Agility));
                case 4: return static_cast<int>(stats->getBaseAttribute(ESM::Attribute::Speed));
                case 5: return static_cast<int>(stats->getBaseAttribute(ESM::Attribute::Endurance));
                case 6: return static_cast<int>(stats->getBaseAttribute(ESM::Attribute::Personality));
                case 7: return static_cast<int>(stats->getBaseAttribute(ESM::Attribute::Luck));
                default: return QVariant(); // throw an exception here?
            }
        }
        else
            switch (subRowIndex)
            {
                case 0: return static_cast<int>(npcStruct.mStrength);
                case 1: return static_cast<int>(npcStruct.mIntelligence);
                case 2: return static_cast<int>(npcStruct.mWillpower);
                case 3: return static_cast<int>(npcStruct.mAgility);
                case 4: return static_cast<int>(npcStruct.mSpeed);
                case 5: return static_cast<int>(npcStruct.mEndurance);
                case 6: return static_cast<int>(npcStruct.mPersonality);
                case 7: return static_cast<int>(npcStruct.mLuck);
                default: return QVariant(); // throw an exception here?
            }
    else
        return QVariant(); // throw an exception here?
}

void CSMWorld::NpcAttributesRefIdAdapter::setNestedData (const RefIdColumn *column,
        RefIdData& data, int row, const QVariant& value, int subRowIndex, int subColIndex) const
{
    Record<ESM::NPC>& record =
        static_cast<Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (row, UniversalId::Type_Npc)));
    ESM::NPC npc = record.get();
    ESM::NPC::NPDTstruct52& npcStruct = npc.mNpdt52;

    if (subColIndex == 1)
        switch(subRowIndex)
        {
            case 0: npcStruct.mStrength = static_cast<unsigned char>(value.toInt()); break;
            case 1: npcStruct.mIntelligence = static_cast<unsigned char>(value.toInt()); break;
            case 2: npcStruct.mWillpower = static_cast<unsigned char>(value.toInt()); break;
            case 3: npcStruct.mAgility = static_cast<unsigned char>(value.toInt()); break;
            case 4: npcStruct.mSpeed = static_cast<unsigned char>(value.toInt()); break;
            case 5: npcStruct.mEndurance = static_cast<unsigned char>(value.toInt()); break;
            case 6: npcStruct.mPersonality = static_cast<unsigned char>(value.toInt()); break;
            case 7: npcStruct.mLuck = static_cast<unsigned char>(value.toInt()); break;
            default: return; // throw an exception here?
        }
    else
        return; // throw an exception here?

    record.setModified (npc);
}

int CSMWorld::NpcAttributesRefIdAdapter::getNestedColumnsCount(const RefIdColumn *column, const RefIdData& data) const
{
    return 2;
}

int CSMWorld::NpcAttributesRefIdAdapter::getNestedRowsCount(const RefIdColumn *column, const RefIdData& data, int index) const
{
    // There are 8 attributes
    return 8;
}

CSMWorld::NpcSkillsRefIdAdapter::NpcSkillsRefIdAdapter(const CSMWorld::Data& data)
    : mData(data)
{}

void CSMWorld::NpcSkillsRefIdAdapter::addNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int position) const
{
    // Do nothing, this table cannot be changed by the user
}

void CSMWorld::NpcSkillsRefIdAdapter::removeNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int rowToRemove) const
{
    // Do nothing, this table cannot be changed by the user
}

void CSMWorld::NpcSkillsRefIdAdapter::setNestedTable (const RefIdColumn* column,
        RefIdData& data, int index, const NestedTableWrapperBase& nestedTable) const
{
    Record<ESM::NPC>& record =
        static_cast<Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Npc)));
    ESM::NPC npc = record.get();

    // store the whole struct
    npc.mNpdt52 =
        static_cast<const NestedTableWrapper<std::vector<typename ESM::NPC::NPDTstruct52> > &>(nestedTable).mNestedTable.at(0);

    record.setModified (npc);
}

CSMWorld::NestedTableWrapperBase* CSMWorld::NpcSkillsRefIdAdapter::nestedTable (const RefIdColumn* column,
        const RefIdData& data, int index) const
{
    const Record<ESM::NPC>& record =
        static_cast<const Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Npc)));

    // return the whole struct
    std::vector<typename ESM::NPC::NPDTstruct52> wrap;
    wrap.push_back(record.get().mNpdt52);
    // deleted by dtor of NestedTableStoring
    return new NestedTableWrapper<std::vector<typename ESM::NPC::NPDTstruct52> >(wrap);
}

QVariant CSMWorld::NpcSkillsRefIdAdapter::getNestedData (const RefIdColumn *column,
        const RefIdData& data, int index, int subRowIndex, int subColIndex) const
{
    const Record<ESM::NPC>& record =
        static_cast<const Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Npc)));

    const ESM::NPC npc = record.get();

    if (subRowIndex < 0 || subRowIndex >= ESM::Skill::Length)
        throw std::runtime_error ("index out of range");

    if (subColIndex == 0)
        return subRowIndex;
    else if (subColIndex == 1)
    {
        if (npc.mNpdtType == ESM::NPC::NPC_WITH_AUTOCALCULATED_STATS)
        {
            CSMWorld::NpcStats *stats =  mData.npcAutoCalculate(npc);
            if (!stats)
                return QVariant();

            return static_cast<int>(stats->getBaseSkill(subRowIndex));
        }
        else
        {
            const ESM::NPC::NPDTstruct52& npcStruct = npc.mNpdt52;
            return static_cast<int>(npcStruct.mSkills[subRowIndex]);
        }
    }
    else
        return QVariant(); // throw an exception here?
}

void CSMWorld::NpcSkillsRefIdAdapter::setNestedData (const RefIdColumn *column,
        RefIdData& data, int row, const QVariant& value, int subRowIndex, int subColIndex) const
{
    Record<ESM::NPC>& record =
        static_cast<Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (row, UniversalId::Type_Npc)));
    ESM::NPC npc = record.get();
    ESM::NPC::NPDTstruct52& npcStruct = npc.mNpdt52;

    if (subRowIndex < 0 || subRowIndex >= ESM::Skill::Length)
        throw std::runtime_error ("index out of range");

    if (subColIndex == 1)
        npcStruct.mSkills[subRowIndex] = static_cast<unsigned char>(value.toInt());
    else
        return; // throw an exception here?

    record.setModified (npc);
}

int CSMWorld::NpcSkillsRefIdAdapter::getNestedColumnsCount(const RefIdColumn *column, const RefIdData& data) const
{
    return 2;
}

int CSMWorld::NpcSkillsRefIdAdapter::getNestedRowsCount(const RefIdColumn *column, const RefIdData& data, int index) const
{
    // There are 27 skills
    return ESM::Skill::Length;
}

CSMWorld::NpcMiscRefIdAdapter::NpcMiscRefIdAdapter(const CSMWorld::Data& data) : mData(data)
{}

CSMWorld::NpcMiscRefIdAdapter::~NpcMiscRefIdAdapter()
{}

void CSMWorld::NpcMiscRefIdAdapter::addNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int position) const
{
    throw std::logic_error ("cannot add a row to a fixed table");
}

void CSMWorld::NpcMiscRefIdAdapter::removeNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int rowToRemove) const
{
    throw std::logic_error ("cannot remove a row to a fixed table");
}

void CSMWorld::NpcMiscRefIdAdapter::setNestedTable (const RefIdColumn* column,
        RefIdData& data, int index, const NestedTableWrapperBase& nestedTable) const
{
    throw std::logic_error ("table operation not supported");
}

CSMWorld::NestedTableWrapperBase* CSMWorld::NpcMiscRefIdAdapter::nestedTable (const RefIdColumn* column,
        const RefIdData& data, int index) const
{
    throw std::logic_error ("table operation not supported");
}

QVariant CSMWorld::NpcMiscRefIdAdapter::getNestedData (const RefIdColumn *column,
        const RefIdData& data, int index, int subRowIndex, int subColIndex) const
{
    const Record<ESM::NPC>& record =
        static_cast<const Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Npc)));

    const ESM::NPC npc = record.get();

    bool autoCalc = (npc.mFlags & ESM::NPC::Autocalc) != 0;

    if (autoCalc)
    {
        CSMWorld::NpcStats *stats =  mData.npcAutoCalculate(npc);

        switch (subColIndex)
        {
            case 0: return static_cast<int>(npc.mNpdt12.mLevel);
            case 1:
            {
                UserInt i(0); // unknown
                return QVariant(QVariant::fromValue(i));
            }
            case 2:
            {
                UserInt i(0);
                if (stats)
                    i = UserInt(stats->getHealth());
                return QVariant(QVariant::fromValue(i));
            }
            case 3:
            {
                UserInt i(0);
                if (stats)
                    i = UserInt(stats->getMana());
                return QVariant(QVariant::fromValue(i));
            }
            case 4:
            {
                UserInt i(0);
                if (stats)
                    i = UserInt(stats->getFatigue());
                return QVariant(QVariant::fromValue(i));
            }
            case 5: return static_cast<int>(record.get().mNpdt12.mDisposition);
            case 6: return static_cast<int>(record.get().mNpdt12.mReputation);
            case 7: return static_cast<int>(record.get().mNpdt12.mRank);
            case 8: return record.get().mNpdt12.mGold;
            case 9: return record.get().mPersistent == true;
            default: return QVariant(); // throw an exception here?
        }
    }
    else
        switch (subColIndex)
        {
            case 0: return static_cast<int>(record.get().mNpdt52.mLevel);
            case 1: return static_cast<int>(record.get().mNpdt52.mFactionID);
            case 2: return static_cast<int>(record.get().mNpdt52.mHealth);
            case 3: return static_cast<int>(record.get().mNpdt52.mMana);
            case 4: return static_cast<int>(record.get().mNpdt52.mFatigue);
            case 5: return static_cast<int>(record.get().mNpdt52.mDisposition);
            case 6: return static_cast<int>(record.get().mNpdt52.mReputation);
            case 7: return static_cast<int>(record.get().mNpdt52.mRank);
            case 8: return record.get().mNpdt52.mGold;
            case 9: return record.get().mPersistent == true;
            default: return QVariant(); // throw an exception here?
        }
}

void CSMWorld::NpcMiscRefIdAdapter::setNestedData (const RefIdColumn *column,
        RefIdData& data, int row, const QVariant& value, int subRowIndex, int subColIndex) const
{
    Record<ESM::NPC>& record =
        static_cast<Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (row, UniversalId::Type_Npc)));
    ESM::NPC npc = record.get();

    bool autoCalc = (record.get().mFlags & ESM::NPC::Autocalc) != 0;

    if (autoCalc)
        switch(subColIndex)
        {
            case 0: npc.mNpdt12.mLevel       = static_cast<short>(value.toInt()); break;
            case 1: return;
            case 2: return;
            case 3: return;
            case 4: return;
            case 5: npc.mNpdt12.mDisposition = static_cast<signed char>(value.toInt()); break;
            case 6: npc.mNpdt12.mReputation  = static_cast<signed char>(value.toInt()); break;
            case 7: npc.mNpdt12.mRank        = static_cast<signed char>(value.toInt()); break;
            case 8: npc.mNpdt12.mGold        = value.toInt(); break;
            case 9: npc.mPersistent          = value.toBool(); break;
            default: return; // throw an exception here?
        }
    else
        switch(subColIndex)
        {
            case 0: npc.mNpdt52.mLevel       = static_cast<short>(value.toInt()); break;
            case 1: npc.mNpdt52.mFactionID   = static_cast<char>(value.toInt()); break;
            case 2: npc.mNpdt52.mHealth      = static_cast<unsigned short>(value.toInt()); break;
            case 3: npc.mNpdt52.mMana        = static_cast<unsigned short>(value.toInt()); break;
            case 4: npc.mNpdt52.mFatigue     = static_cast<unsigned short>(value.toInt()); break;
            case 5: npc.mNpdt52.mDisposition = static_cast<signed char>(value.toInt()); break;
            case 6: npc.mNpdt52.mReputation  = static_cast<signed char>(value.toInt()); break;
            case 7: npc.mNpdt52.mRank        = static_cast<signed char>(value.toInt()); break;
            case 8: npc.mNpdt52.mGold        = value.toInt(); break;
            case 9: npc.mPersistent          = value.toBool(); break;
            default: return; // throw an exception here?
        }

    record.setModified (npc);
}

int CSMWorld::NpcMiscRefIdAdapter::getNestedColumnsCount(const RefIdColumn *column, const RefIdData& data) const
{
    return 10; // Level, FactionID, Health, Mana, Fatigue, Disposition, Reputation, Rank, Gold, Persist
}

int CSMWorld::NpcMiscRefIdAdapter::getNestedRowsCount(const RefIdColumn *column, const RefIdData& data, int index) const
{
    return 1; // fixed at size 1
}

CSMWorld::CreatureAttributesRefIdAdapter::CreatureAttributesRefIdAdapter()
{}

void CSMWorld::CreatureAttributesRefIdAdapter::addNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int position) const
{
    // Do nothing, this table cannot be changed by the user
}

void CSMWorld::CreatureAttributesRefIdAdapter::removeNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int rowToRemove) const
{
    // Do nothing, this table cannot be changed by the user
}

void CSMWorld::CreatureAttributesRefIdAdapter::setNestedTable (const RefIdColumn* column,
        RefIdData& data, int index, const NestedTableWrapperBase& nestedTable) const
{
    Record<ESM::Creature>& record =
        static_cast<Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Creature)));
    ESM::Creature creature = record.get();

    // store the whole struct
    creature.mData =
        static_cast<const NestedTableWrapper<std::vector<typename ESM::Creature::NPDTstruct> > &>(nestedTable).mNestedTable.at(0);

    record.setModified (creature);
}

CSMWorld::NestedTableWrapperBase* CSMWorld::CreatureAttributesRefIdAdapter::nestedTable (const RefIdColumn* column,
        const RefIdData& data, int index) const
{
    const Record<ESM::Creature>& record =
        static_cast<const Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Creature)));

    // return the whole struct
    std::vector<typename ESM::Creature::NPDTstruct> wrap;
    wrap.push_back(record.get().mData);
    // deleted by dtor of NestedTableStoring
    return new NestedTableWrapper<std::vector<typename ESM::Creature::NPDTstruct> >(wrap);
}

QVariant CSMWorld::CreatureAttributesRefIdAdapter::getNestedData (const RefIdColumn *column,
        const RefIdData& data, int index, int subRowIndex, int subColIndex) const
{
    const Record<ESM::Creature>& record =
        static_cast<const Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Creature)));

    const ESM::Creature creature = record.get();

    if (subColIndex == 0)
        return subRowIndex;
    else if (subColIndex == 1)
        switch (subRowIndex)
        {
            case 0: return creature.mData.mStrength;
            case 1: return creature.mData.mIntelligence;
            case 2: return creature.mData.mWillpower;
            case 3: return creature.mData.mAgility;
            case 4: return creature.mData.mSpeed;
            case 5: return creature.mData.mEndurance;
            case 6: return creature.mData.mPersonality;
            case 7: return creature.mData.mLuck;
            default: return QVariant(); // throw an exception here?
        }
    else
        return QVariant(); // throw an exception here?
}

void CSMWorld::CreatureAttributesRefIdAdapter::setNestedData (const RefIdColumn *column,
        RefIdData& data, int row, const QVariant& value, int subRowIndex, int subColIndex) const
{
    Record<ESM::Creature>& record =
        static_cast<Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (row, UniversalId::Type_Creature)));
    ESM::Creature creature = record.get();

    if (subColIndex == 1)
        switch(subRowIndex)
        {
            case 0: creature.mData.mStrength = value.toInt(); break;
            case 1: creature.mData.mIntelligence = value.toInt(); break;
            case 2: creature.mData.mWillpower = value.toInt(); break;
            case 3: creature.mData.mAgility = value.toInt(); break;
            case 4: creature.mData.mSpeed = value.toInt(); break;
            case 5: creature.mData.mEndurance = value.toInt(); break;
            case 6: creature.mData.mPersonality = value.toInt(); break;
            case 7: creature.mData.mLuck = value.toInt(); break;
            default: return; // throw an exception here?
        }
    else
        return; // throw an exception here?

    record.setModified (creature);
}

int CSMWorld::CreatureAttributesRefIdAdapter::getNestedColumnsCount(const RefIdColumn *column, const RefIdData& data) const
{
    return 2;
}

int CSMWorld::CreatureAttributesRefIdAdapter::getNestedRowsCount(const RefIdColumn *column, const RefIdData& data, int index) const
{
    // There are 8 attributes
    return 8;
}

CSMWorld::CreatureAttackRefIdAdapter::CreatureAttackRefIdAdapter()
{}

void CSMWorld::CreatureAttackRefIdAdapter::addNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int position) const
{
    // Do nothing, this table cannot be changed by the user
}

void CSMWorld::CreatureAttackRefIdAdapter::removeNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int rowToRemove) const
{
    // Do nothing, this table cannot be changed by the user
}

void CSMWorld::CreatureAttackRefIdAdapter::setNestedTable (const RefIdColumn* column,
        RefIdData& data, int index, const NestedTableWrapperBase& nestedTable) const
{
    Record<ESM::Creature>& record =
        static_cast<Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Creature)));
    ESM::Creature creature = record.get();

    // store the whole struct
    creature.mData =
        static_cast<const NestedTableWrapper<std::vector<typename ESM::Creature::NPDTstruct> > &>(nestedTable).mNestedTable.at(0);

    record.setModified (creature);
}

CSMWorld::NestedTableWrapperBase* CSMWorld::CreatureAttackRefIdAdapter::nestedTable (const RefIdColumn* column,
        const RefIdData& data, int index) const
{
    const Record<ESM::Creature>& record =
        static_cast<const Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Creature)));

    // return the whole struct
    std::vector<typename ESM::Creature::NPDTstruct> wrap;
    wrap.push_back(record.get().mData);
    // deleted by dtor of NestedTableStoring
    return new NestedTableWrapper<std::vector<typename ESM::Creature::NPDTstruct> >(wrap);
}

QVariant CSMWorld::CreatureAttackRefIdAdapter::getNestedData (const RefIdColumn *column,
        const RefIdData& data, int index, int subRowIndex, int subColIndex) const
{
    const Record<ESM::Creature>& record =
        static_cast<const Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Creature)));

    const ESM::Creature creature = record.get();

    if (subRowIndex < 0 || subRowIndex > 2 || subColIndex < 0 || subColIndex > 2)
        throw std::runtime_error ("index out of range");

    if (subColIndex == 0)
        return subRowIndex + 1;
    else if (subColIndex < 3) // 1 or 2
        return creature.mData.mAttack[(subRowIndex * 2) + (subColIndex - 1)];
    else
        return QVariant(); // throw an exception here?
}

void CSMWorld::CreatureAttackRefIdAdapter::setNestedData (const RefIdColumn *column,
        RefIdData& data, int row, const QVariant& value, int subRowIndex, int subColIndex) const
{
    Record<ESM::Creature>& record =
        static_cast<Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (row, UniversalId::Type_Creature)));
    ESM::Creature creature = record.get();

    if (subRowIndex < 0 || subRowIndex > 2)
        throw std::runtime_error ("index out of range");

    if (subColIndex == 1 || subColIndex == 2)
        creature.mData.mAttack[(subRowIndex * 2) + (subColIndex - 1)] = value.toInt();
    else
        return; // throw an exception here?

    record.setModified (creature);
}

int CSMWorld::CreatureAttackRefIdAdapter::getNestedColumnsCount(const RefIdColumn *column, const RefIdData& data) const
{
    return 3;
}

int CSMWorld::CreatureAttackRefIdAdapter::getNestedRowsCount(const RefIdColumn *column, const RefIdData& data, int index) const
{
    // There are 3 attacks
    return 3;
}

CSMWorld::CreatureMiscRefIdAdapter::CreatureMiscRefIdAdapter()
{}

CSMWorld::CreatureMiscRefIdAdapter::~CreatureMiscRefIdAdapter()
{}

void CSMWorld::CreatureMiscRefIdAdapter::addNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int position) const
{
    throw std::logic_error ("cannot add a row to a fixed table");
}

void CSMWorld::CreatureMiscRefIdAdapter::removeNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int rowToRemove) const
{
    throw std::logic_error ("cannot remove a row to a fixed table");
}

void CSMWorld::CreatureMiscRefIdAdapter::setNestedTable (const RefIdColumn* column,
        RefIdData& data, int index, const NestedTableWrapperBase& nestedTable) const
{
    throw std::logic_error ("table operation not supported");
}

CSMWorld::NestedTableWrapperBase* CSMWorld::CreatureMiscRefIdAdapter::nestedTable (const RefIdColumn* column,
        const RefIdData& data, int index) const
{
    throw std::logic_error ("table operation not supported");
}

QVariant CSMWorld::CreatureMiscRefIdAdapter::getNestedData (const RefIdColumn *column,
        const RefIdData& data, int index, int subRowIndex, int subColIndex) const
{
    const Record<ESM::Creature>& record =
        static_cast<const Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Creature)));

    const ESM::Creature creature = record.get();

    switch (subColIndex)
    {
        case 0: return creature.mData.mLevel;
        case 1: return creature.mData.mHealth;
        case 2: return creature.mData.mMana;
        case 3: return creature.mData.mFatigue;
        case 4: return creature.mData.mSoul;
        case 5: return creature.mData.mCombat;
        case 6: return creature.mData.mMagic;
        case 7: return creature.mData.mStealth;
        case 8: return creature.mData.mGold;
        default: return QVariant(); // throw an exception here?
    }
}

void CSMWorld::CreatureMiscRefIdAdapter::setNestedData (const RefIdColumn *column,
        RefIdData& data, int row, const QVariant& value, int subRowIndex, int subColIndex) const
{
    Record<ESM::Creature>& record =
        static_cast<Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (row, UniversalId::Type_Creature)));
    ESM::Creature creature = record.get();

    switch(subColIndex)
    {
        case 0: creature.mData.mLevel   = value.toInt(); break;
        case 1: creature.mData.mHealth  = value.toInt(); break;
        case 2: creature.mData.mMana    = value.toInt(); break;
        case 3: creature.mData.mFatigue = value.toInt(); break;
        case 4: creature.mData.mSoul    = value.toInt(); break;
        case 5: creature.mData.mCombat  = value.toInt(); break;
        case 6: creature.mData.mMagic   = value.toInt(); break;
        case 7: creature.mData.mStealth = value.toInt(); break;
        case 8: creature.mData.mGold    = value.toInt(); break;
        default: return; // throw an exception here?
    }

    record.setModified (creature);
}

int CSMWorld::CreatureMiscRefIdAdapter::getNestedColumnsCount(const RefIdColumn *column, const RefIdData& data) const
{
    return 9; // Level, Health, Mana, Fatigue, Soul, Combat, Magic, Steath, Gold
}

int CSMWorld::CreatureMiscRefIdAdapter::getNestedRowsCount(const RefIdColumn *column, const RefIdData& data, int index) const
{
    return 1; // fixed at size 1
}

CSMWorld::WeaponColumns::WeaponColumns (const EnchantableColumns& columns)
: EnchantableColumns (columns) {}

CSMWorld::WeaponRefIdAdapter::WeaponRefIdAdapter (const WeaponColumns& columns)
: EnchantableRefIdAdapter<ESM::Weapon> (UniversalId::Type_Weapon, columns), mColumns (columns)
{}

QVariant CSMWorld::WeaponRefIdAdapter::getData (const RefIdColumn *column, const RefIdData& data,
    int index) const
{
    const Record<ESM::Weapon>& record = static_cast<const Record<ESM::Weapon>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Weapon)));

    if (column==mColumns.mType)
        return record.get().mData.mType;

    if (column==mColumns.mHealth)
        return record.get().mData.mHealth;

    if (column==mColumns.mSpeed)
        return record.get().mData.mSpeed;

    if (column==mColumns.mReach)
        return record.get().mData.mReach;

    for (int i=0; i<2; ++i)
    {
        if (column==mColumns.mChop[i])
            return record.get().mData.mChop[i];

        if (column==mColumns.mSlash[i])
            return record.get().mData.mSlash[i];

        if (column==mColumns.mThrust[i])
            return record.get().mData.mThrust[i];
    }

    std::map<const RefIdColumn *, unsigned int>::const_iterator iter =
        mColumns.mFlags.find (column);

    if (iter!=mColumns.mFlags.end())
        return (record.get().mData.mFlags & iter->second)!=0;

    return EnchantableRefIdAdapter<ESM::Weapon>::getData (column, data, index);
}

void CSMWorld::WeaponRefIdAdapter::setData (const RefIdColumn *column, RefIdData& data, int index,
    const QVariant& value) const
{
    Record<ESM::Weapon>& record = static_cast<Record<ESM::Weapon>&> (
        data.getRecord (RefIdData::LocalIndex (index, UniversalId::Type_Weapon)));

    if (column==mColumns.mType)
        record.get().mData.mType = value.toInt();
    else if (column==mColumns.mHealth)
        record.get().mData.mHealth = value.toInt();
    else if (column==mColumns.mSpeed)
        record.get().mData.mSpeed = value.toFloat();
    else if (column==mColumns.mReach)
        record.get().mData.mReach = value.toFloat();
    else if (column==mColumns.mChop[0])
        record.get().mData.mChop[0] = value.toInt();
    else if (column==mColumns.mChop[1])
        record.get().mData.mChop[1] = value.toInt();
    else if (column==mColumns.mSlash[0])
        record.get().mData.mSlash[0] = value.toInt();
    else if (column==mColumns.mSlash[1])
        record.get().mData.mSlash[1] = value.toInt();
    else if (column==mColumns.mThrust[0])
        record.get().mData.mThrust[0] = value.toInt();
    else if (column==mColumns.mThrust[1])
        record.get().mData.mThrust[1] = value.toInt();
    else
    {
        std::map<const RefIdColumn *, unsigned int>::const_iterator iter =
            mColumns.mFlags.find (column);

        if (iter!=mColumns.mFlags.end())
        {
            if (value.toInt()!=0)
                record.get().mData.mFlags |= iter->second;
            else
                record.get().mData.mFlags &= ~iter->second;
        }
        else
            EnchantableRefIdAdapter<ESM::Weapon>::setData (column, data, index, value);
    }
}

namespace CSMWorld
{

template<>
QVariant ActorRefIdAdapter<ESM::NPC>::getData (const RefIdColumn *column, const RefIdData& data,
    int index) const
{
    const Record<ESM::NPC>& record = static_cast<const Record<ESM::NPC>&> (
        data.getRecord (RefIdData::LocalIndex (index, BaseRefIdAdapter<ESM::NPC>::getType())));

    if (column==mActors.mHello)
        return record.get().mAiData.mHello;

    if (column==mActors.mFlee)
        return record.get().mAiData.mFlee;

    if (column==mActors.mFight)
        return record.get().mAiData.mFight;

    if (column==mActors.mAlarm)
        return record.get().mAiData.mAlarm;

    if (column==mActors.mInventory)
        return true; // to show nested tables in dialogue subview, see IdTree::hasChildren()

    if (column==mActors.mSpells)
    {
        if ((record.get().mFlags & ESM::NPC::Autocalc) != 0)
            return QVariant(QVariant::UserType);
        else
            return true;
    }

    if (column==mActors.mDestinations)
        return true; // to show nested tables in dialogue subview, see IdTree::hasChildren()

    if (column==mActors.mAiPackages)
        return true; // to show nested tables in dialogue subview, see IdTree::hasChildren()

    std::map<const RefIdColumn *, unsigned int>::const_iterator iter =
        mActors.mServices.find (column);

    if (iter!=mActors.mServices.end())
        return (record.get().mAiData.mServices & iter->second)!=0;

    return NameRefIdAdapter<ESM::NPC>::getData (column, data, index);
}

template <>
void NestedSpellRefIdAdapter<ESM::NPC>::addNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int position) const
{
    Record<ESM::NPC>& record =
        static_cast<Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));

    if (record.get().mNpdtType == ESM::NPC::NPC_WITH_AUTOCALCULATED_STATS)
        return; // can't edit autocalculated spells

    ESM::NPC caster = record.get();

    std::vector<std::string>& list = caster.mSpells.mList;

    std::string newString;

    if (position >= (int)list.size())
        list.push_back(newString);
    else
        list.insert(list.begin()+position, newString);

    record.setModified (caster);
}

template <>
void NestedSpellRefIdAdapter<ESM::NPC>::removeNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int rowToRemove) const
{
    Record<ESM::NPC>& record =
        static_cast<Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));

    if (record.get().mNpdtType == ESM::NPC::NPC_WITH_AUTOCALCULATED_STATS)
        return; // can't edit autocalculated spells

    ESM::NPC caster = record.get();

    std::vector<std::string>& list = caster.mSpells.mList;

    // avoid race power rows
    int size = 0;
    int raceIndex = mData.getRaces().searchId(caster.mRace);
    if (raceIndex != -1)
        size = mData.getRaces().getRecord(raceIndex).get().mPowers.mList.size();

    if (rowToRemove < 0 || rowToRemove >= static_cast<int> (list.size() + size))
        throw std::runtime_error ("index out of range");

    if (rowToRemove >= static_cast<int>(list.size()) && rowToRemove < static_cast<int>(list.size() + size))
        return; // hack, assumes the race powers are added at the end

    list.erase (list.begin () + rowToRemove);

    record.setModified (caster);
}

template <>
void NestedSpellRefIdAdapter<ESM::NPC>::setNestedData (const RefIdColumn *column,
        RefIdData& data, int row, const QVariant& value, int subRowIndex, int subColIndex) const
{
    Record<ESM::NPC>& record =
        static_cast<Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (row, mType)));

    if (record.get().mNpdtType == ESM::NPC::NPC_WITH_AUTOCALCULATED_STATS)
        return; // can't edit autocalculated spells

    ESM::NPC caster = record.get();
    std::vector<std::string>& list = caster.mSpells.mList;

    // avoid race power rows
    int size = 0;
    int raceIndex = mData.getRaces().searchId(caster.mRace);
    if (raceIndex != -1)
        size = mData.getRaces().getRecord(raceIndex).get().mPowers.mList.size();

    if (subRowIndex < 0 || subRowIndex >= static_cast<int> (list.size() + size))
        throw std::runtime_error ("index out of range");

    if (subRowIndex >= static_cast<int>(list.size()) && subRowIndex < static_cast<int>(list.size() + size))
        return; // hack, assumes the race powers are added at the end

    if (subColIndex == 0)
        list.at(subRowIndex) = std::string(value.toString().toUtf8());
    else
        throw std::runtime_error("Trying to access non-existing column in the nested table!");

    record.setModified (caster);
}

template <>
QVariant NestedSpellRefIdAdapter<ESM::NPC>::getNestedData (const RefIdColumn *column,
        const RefIdData& data, int index, int subRowIndex, int subColIndex) const
{
    const Record<ESM::NPC>& record =
        static_cast<const Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));

    CSMWorld::NpcStats *stats = mData.npcAutoCalculate(record.get());
    if (!stats)
        return QVariant();

    const std::vector<SpellInfo>& spells = stats->spells();

    if (subRowIndex < 0 || subRowIndex >= static_cast<int> (spells.size()))
        throw std::runtime_error ("index out of range");

    switch (subColIndex)
    {
        case 0: return QString::fromUtf8(spells[subRowIndex].mName.c_str());
        case 1: return spells[subRowIndex].mType;
        case 2: return spells[subRowIndex].mFromRace;
        case 3: return spells[subRowIndex].mCost;
        case 4: return spells[subRowIndex].mChance;
        default:
            throw std::runtime_error("Trying to access non-existing column in the nested table!");
    }
}

template <>
int NestedSpellRefIdAdapter<ESM::NPC>::getNestedColumnsCount(const RefIdColumn *column,
        const RefIdData& data) const
{
    return 5;
}

template <>
int NestedSpellRefIdAdapter<ESM::NPC>::getNestedRowsCount(const RefIdColumn *column, const RefIdData& data, int index) const
{
    const Record<ESM::NPC>& record =
        static_cast<const Record<ESM::NPC>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));

    CSMWorld::NpcStats *stats = mData.npcAutoCalculate(record.get());
    if (!stats)
        return 0;

    return static_cast<int>(stats->spells().size());
}

template<>
QVariant ActorRefIdAdapter<ESM::Creature>::getData (const RefIdColumn *column, const RefIdData& data,
    int index) const
{
    const Record<ESM::Creature>& record = static_cast<const Record<ESM::Creature>&> (
        data.getRecord (RefIdData::LocalIndex (index, BaseRefIdAdapter<ESM::Creature>::getType())));

    if (column==mActors.mHello)
        return record.get().mAiData.mHello;

    if (column==mActors.mFlee)
        return record.get().mAiData.mFlee;

    if (column==mActors.mFight)
        return record.get().mAiData.mFight;

    if (column==mActors.mAlarm)
        return record.get().mAiData.mAlarm;

    if (column==mActors.mInventory)
        return true; // to show nested tables in dialogue subview, see IdTree::hasChildren()

    if (column==mActors.mSpells)
        return true; // to show nested tables in dialogue subview, see IdTree::hasChildren()

    if (column==mActors.mDestinations)
        return true; // to show nested tables in dialogue subview, see IdTree::hasChildren()

    if (column==mActors.mAiPackages)
        return true; // to show nested tables in dialogue subview, see IdTree::hasChildren()

    std::map<const RefIdColumn *, unsigned int>::const_iterator iter =
        mActors.mServices.find (column);

    if (iter!=mActors.mServices.end())
        return (record.get().mAiData.mServices & iter->second)!=0;

    return NameRefIdAdapter<ESM::Creature>::getData (column, data, index);
}

template <>
void NestedSpellRefIdAdapter<ESM::Creature>::addNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int position) const
{
    Record<ESM::Creature>& record =
        static_cast<Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));
    ESM::Creature caster = record.get();

    std::vector<std::string>& list = caster.mSpells.mList;

    std::string newString;

    if (position >= (int)list.size())
        list.push_back(newString);
    else
        list.insert(list.begin()+position, newString);

    record.setModified (caster);
}

template <>
void NestedSpellRefIdAdapter<ESM::Creature>::removeNestedRow (const RefIdColumn *column,
        RefIdData& data, int index, int rowToRemove) const
{
    Record<ESM::Creature>& record =
        static_cast<Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));
    ESM::Creature caster = record.get();

    std::vector<std::string>& list = caster.mSpells.mList;

    if (rowToRemove < 0 || rowToRemove >= static_cast<int> (list.size()))
        throw std::runtime_error ("index out of range");

    list.erase (list.begin () + rowToRemove);

    record.setModified (caster);
}

template <>
void NestedSpellRefIdAdapter<ESM::Creature>::setNestedData (const RefIdColumn *column,
        RefIdData& data, int row, const QVariant& value, int subRowIndex, int subColIndex) const
{
    Record<ESM::Creature>& record =
        static_cast<Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (row, mType)));
    ESM::Creature caster = record.get();
    std::vector<std::string>& list = caster.mSpells.mList;

    if (subRowIndex < 0 || subRowIndex >= static_cast<int> (list.size()))
        throw std::runtime_error ("index out of range");

    if (subColIndex == 0)
        list.at(subRowIndex) = std::string(value.toString().toUtf8());
    else
        throw std::runtime_error("Trying to access non-existing column in the nested table!");

    record.setModified (caster);
}

template<>
QVariant NestedSpellRefIdAdapter<ESM::Creature>::getNestedData (const RefIdColumn *column,
        const RefIdData& data, int index, int subRowIndex, int subColIndex) const
{
    const Record<ESM::Creature>& record =
        static_cast<const Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));

    const std::vector<std::string>& list = record.get().mSpells.mList;

    if (subRowIndex < 0 || subRowIndex >= static_cast<int> (list.size()))
        throw std::runtime_error ("index out of range");

    const std::string& content = list.at(subRowIndex);

    int type = -1;
    int spellIndex = mData.getSpells().searchId(content);
    if (spellIndex != -1)
        type = mData.getSpells().getRecord(spellIndex).get().mData.mType;

    if (subColIndex == 0)
        return QString::fromUtf8(content.c_str());
    else if (subColIndex == 1)
        return type;
    else
        throw std::runtime_error("Trying to access non-existing column in the nested table!");
}

template <>
int NestedSpellRefIdAdapter<ESM::Creature>::getNestedColumnsCount(const RefIdColumn *column,
        const RefIdData& data) const
{
    return 2;
}

template <>
int NestedSpellRefIdAdapter<ESM::Creature>::getNestedRowsCount(const RefIdColumn *column, const RefIdData& data, int index) const
{
    const Record<ESM::Creature>& record =
        static_cast<const Record<ESM::Creature>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));

    return static_cast<int>(record.get().mSpells.mList.size());
}

}
