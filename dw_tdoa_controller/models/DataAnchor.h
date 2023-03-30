// -------------------------------------------------------------------------------------------------------------------
//
//  File: DataAnchor.h
//
//  Copyright 2014 (c) DecaWave Ltd, Dublin, Ireland.
//
//  All rights reserved.
//
//  Author:
//
// -------------------------------------------------------------------------------------------------------------------

#ifndef DATAANCHOR_H
#define DATAANCHOR_H

#include "DataAbstractItem.h"

#define SECONDARY_MASTER (2)
#define PRIMARY_MASTER   (1)
#define SLAVE            (0)

class QVariant;
class DataModel;
class DataLink;
class DataUndirectedLink;

typedef enum {
    NotConnected = 0,
    EthConnected = 1,
    UwbBhConnected = 2
}_connected_e;

struct ReferencePair
{
    quint64 anchorid;   ///< reference anchor's ID
    quint64 tagid;      ///< reference tag's ID
    quint64 offset;     ///< 5 byte DW1000 time base...
    double tagx;        ///< reference tag's x co-ordinate
    double tagy;        ///< reference tag's y co-ordinate
    double tagz;        ///< reference tag's z co-ordinate
    int blinks;         ///< number of blinks to use for training
};

/**
 * The DataAnchor class represents an anchor within the model.
 *
 * It stores the ID, XYZ coordinates and whether the anchor is a clock sync master.
 * Its child items are DataLink%s
 */
class DataAnchor : public DataAbstractItem
{
public:
    enum Column {
        ColumnID = 0,    ///< 64 bit address of the anchor (uint64)
        ColumnNumber,    ///< anchor number in the RTLS (int)
        ColumnX,         ///< X coordinate (double)
        ColumnY,         ///< Y coordinate (double)
        ColumnZ,         ///< Z coordinate (double)
        ColumnMasterSlave,    ///< 0 = slave, 1 = primary, 2 = secondary master (int)
        ColumnSelected,  ///< true if this anchor is to be connected to CLE
        ColumnConnected, ///< true if this anchor is connected to CLE (status of connection)
        ColumnDlyRx,     ///< rx antenna delay value in ns (double)
        ColumnDlyTx,     ///< tx antenna delay value in ns (double)
        ColumnMasterID,  ///< Primary master ID (uint64)
        ColumnLagDelay,  ///< the lag this secondary master (if master) will send its CCPs after reception of primary's CCPs (double)
        ColumnDim,      ///< Anchor type 1 = 1D anchor ; 2 = 2D anchor (int) >
        ColumnCount
    };

    /**
     * Construct a new DataAnchor object.
     * You shouldn't create DataAnchor objects directly. Use DataModel::anchor() instead.
     *
     * @param model the model in which contains this item.
     * @param parent the item's parent
     * @param row the item's row index, within the parent
     * @param id the anchor's 64 bit ID
     */
    explicit DataAnchor(DataModel *model, DataAbstractItem *parent, int row, uint64_t id);
    virtual ~DataAnchor();

    /// @{
    virtual Type type();

    virtual unsigned int rowCount() const;
    virtual unsigned int columnCount() const;

    virtual QVariant data(int column) const;
    virtual bool setData(int column, const QVariant &data);

    virtual DataAbstractItem *child(int row);
    virtual const DataAbstractItem *child(int row) const;

    virtual bool isEditable(int column) const;
    /// @}

    /**
     * @param dim present the type of anchor.
     */
    void setDim(int dim);

    /**
     * @param x the new number value.
     */
    void setNumber(int x);

    /**
     * @param x the new X value.
     */
    void setX(double x);

    /**
     * @param y the new Y value.
     */
    void setY(double y);

    /**
     * @param z the new Z value.
     */
    void setZ(double z);

    /**
     * Change the X and Y coordinates.
     * @param x the new X value.
     * @param y the new Y value.
     */
    void setCoordinates(double x, double y);

    /**
     * Change the coordinates.
     * @param x the new X value.
     * @param y the new Y value.
     * @param z the new Z value.
     */
    void setCoordinates(double x, double y, double z);

    /**
     * @param master true if the anchor is a master, false otherwise.
     */
    void setMaster(int master);

    /**
     * @param selected true if the anchor is selected, false otherwise.
     */
    void setSelected(bool selected);

    /**
     * @param connected true if the anchor is connected to the CLE, false otherwise.
     */
    void setConnected(_connected_e connected);

    /**
     * @param rx dly the new antenna delay value.
     */
    void setAntennaDlyRx(double dly);

    /**
     * @param tx dly the new antenna delay value.
     */
    void setAntennaDlyTx(double dly);

    /**
     * @return the 64 bit address
     */
    uint64_t id() const { return _id; }

    /**
     * @return anchor's X
     */
    double x() const { return _x; }

    /**
     * @return anchor's Y
     */
    double y() const { return _y; }

    /**
     * @return anchor's Z
     */
    double z() const { return _z; }

    /**
     * @return PRIMARY_MASTER if the anchor is a primary master, SECONDARY_MASTER is secondary else SLAVE if slave
     */
    int master() const { return _master; }

    /**
     * @return true if the anchor is selected (if CLE should connect(or is connected) to it), false otherwise
     */
    bool selected() const { return _selected; }

    /**
     * @return true if the anchor is connected to the CLE
     */
    bool connected() const { return _connected; }

    /**
     * @return anchor's antenna rx dly
     */
    double antennaDlyRx() const { return _antennaDlyRx; }

    /**
     * @return anchor's antenna tx dly
     */
    double antennaDlyTx() const { return _antennaDlyTx; }

    /**
     * @return anchor's number
     */
    int number() const { return _number; }

    /**
     * @return anchor's type
     */
    int anchorDim() const { return _dim; }

    /**
     * save the current anchor's x, y, and z coordinates
     */
    void saveCoords() { _xs = _x; _ys = _y; _zs = _z;}

    /**
     * restore the saved anchor's x, y, and z coordinates
     */
    void restoreCoords() { _x = _xs; _y = _ys; _z = _zs;}

    bool position() {return _position;}
    void setPosition(bool value){_position = value;}

    /**
     * @return the 64 bit address of the primary master
     */
    uint64_t masterId() const { return _masterId; }

    /**
     * @param the new 64 bit address of the primary master
     */
    void setMasterId(uint64_t id);

    /**
     * @return the 64 bit address of the primary master
     */
    int lagDelayUs() const { return _lagDelayUs; }

    /**
     * @param the new 64 bit address of the primary master
     */
    void setLagDelayUs(int lag);

    /**
     * Get the link to the specified target anchor.
     * @param target the target anchor to look for
     * @return the DataLink object or nullptr if it doesn't exist
     */
    DataLink *link(DataAnchor *target);

    /**
     * Add a master anchor to the master anchor list
     * @param the 64 bit address of the master anchor to add
     */
    void addMasterAnchor(uint64_t id);

    /**
     * Get a master anchor ID based on the array index passed
     * @param the index in the master anchor list whose master ID we want to get
     * @return the 64 bit address of the master anchor to add
     */
    uint64_t getMasterAnchorAddrAt(int i);

    /**
     * @return the size of the master anchor list
     */
    int masterAnchorListSize() { return _masteranchors.size(); }
    /**
     * clear the contents of the master anchor list
     */
    void clearMasterAnchors()  { _masteranchors.clear(); }

    /**
     * Add a reference pair (anchor, tag) to the reference pair list
     * @param the 64 bit address of the reference anchor to add
     * @param the 64 bit tag address of the reference tag for this anchor
     * @param the x coordinate of the reference tag
     * @param the y coordinate of the reference tag
     * @param the z coordinate of the reference tag
     * @param the offset
     */
    void addReferencePair(uint64_t id, uint64_t tagid, double x, double y, double z, int offset, int blinks);

    /**
     * Get a reference pair based on the array index passed
     * @param the index in the reference pair list whose pair we want to get
     * @return the reference pair
     */
    ReferencePair getReferencePairAt(int i);

    /**
     * @return the size of the master anchor list
     */
    int referencePairsListSize() { return _refpairs.size(); }
    /**
     * clear the contents of the master anchor list
     */
    void clearReferencePairs()  { _refpairs.clear(); }

protected:
    /**
     * Add a link to the anchor.
     * The link must not exist already.
     * This is should only be called by the DataUndirectedLink contructor.
     * Other classes should use DataModel::link() for that purpose.
     *
     * @param target the links' target
     * @param undirectedLink the shared DataUndirectedLink object
     * @return the newly created link
     */
    DataLink *addLink(DataAnchor *target, DataUndirectedLink *undirectedLink);

    /**
     * Update the child links after changes have been made to the position.
     * Iterates over the links and calls the DataLink::anchorMoved() method to update eg the distance.
     */
    void anchorMoved();

private:
    uint8_t _dim;          // =1 表示1维anchor  =2 表示2维anchor
    uint64_t _id;
    double _x, _y, _z;
    int _master;
    bool _selected; //if true then this anchor is/will be conneted to the CLE
    _connected_e _connected;
    int _number; //this is anchor number (e.g. in 4 anchor system anchors are labeled 1, 2, 3, 4)
    double _antennaDlyRx,_antennaDlyTx;
    double _xs, _ys, _zs;
    bool _position;
    uint64_t _masterId;
    int _lagDelayUs;
    QMap<uint64_t, DataLink *> _links;
    QList<DataLink *> _linkList;
    QList<uint64_t> _masteranchors;
    friend class DataUndirectedLink;

    QList<ReferencePair> _refpairs;
};

#endif // DATAANCHOR_H
