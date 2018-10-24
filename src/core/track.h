/*
 *    Copyright 2012, 2013 Thomas Schöps
 *    Copyright 2017, 2018 Kai Pastor
 *
 *    This file is part of OpenOrienteering.
 *
 *    OpenOrienteering is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    OpenOrienteering is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with OpenOrienteering.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef OPENORIENTEERING_TRACK_H
#define OPENORIENTEERING_TRACK_H

#include <cmath>
#include <vector>

#include <QtNumeric>
#include <QDateTime>
#include <QString>

#include "core/georeferencing.h"
#include "core/latlon.h"
#include "core/map_coord.h"

class QFile;
class QXmlStreamWriter;

namespace OpenOrienteering {


/**
 * A geographic point in a track or a waypoint.
 * 
 * A TrackPoint stores position on ellipsoid and on map, and more attributes.
 */
struct TrackPoint
{
	LatLon gps_coord    = {};
	QDateTime datetime  = {};       // QDateTime() if invalid
	double elevation    = qQNaN();  // NaN if invalid
	float hDOP          = NAN;      // NaN if invalid
	MapCoordF map_coord = {};
	
	// Default special member functions are fine.
	// \todo Remove the special member functions after dropping Android gcc 4.9.
#if __GNUC__ == 4
	TrackPoint() = default;
	TrackPoint(const TrackPoint&) = default;
	TrackPoint(LatLon gps_coord,
	           QDateTime datetime  = {},
	           double elevation    = qQNaN(),
	           float hDOP          = NAN,
	           MapCoordF map_coord = {} )
	: gps_coord(gps_coord)
	, datetime(datetime)
	, elevation(elevation)
	, hDOP(hDOP)
	, map_coord(map_coord)
	{}
	~TrackPoint() = default;
#endif
	
	void save(QXmlStreamWriter* stream) const;
};

/**
 * Stores a set of tracks and / or waypoints, e.g. taken from a GPS device.
 * Can optionally store a track coordinate reference system in track_georef;
 * if no track CRS is given, assumes that coordinates are geographic WGS84 coordinates
 */
class Track
{
public:
	/// Constructs an empty track
	Track() = default;
	
	Track(const Georeferencing& map_georef);
	/// Duplicates a track
	Track(const Track& other);
	
	~Track();
	
	/// Deletes all data of the track, except the projection parameters
	void clear();
	
	/// Attempts to load the track from the given file.
	/// If you choose not to project_point, you have to call changeProjectionParams() afterwards.
	bool loadFrom(const QString& path, bool project_points);
	/// Attempts to save the track to the given file
	bool saveTo(const QString& path) const;
	
	/// Returns the track's CRS specification (PROJ.4 format).
	QString crsSpec() const;
	
	// Modifiers
	
	/**
	 * @brief Appends the point and updates the point's map coordinates.
	 * 
	 * The point's map coordinates are determined from its geographic coodinates
	 * according to the map's georeferencing.
	 */
	void appendTrackPoint(const TrackPoint& point);
	
	/**
	 * Marks the current track segment as finished, so the next added point
	 * will define the start of a new track segment.
	 */
	void finishCurrentSegment();
	
	/**
	 * @brief Appends the waypoint and updates the point's map coordinates.
	 * 
	 * The point's map coordinates are determined from its geographic coodinates
	 * according to the map's georeferencing.
	 */
	void appendWaypoint(const TrackPoint& point, const QString& name);
	
	/** Updates the map positions of all points based on the new georeferencing. */
	void changeMapGeoreferencing(const Georeferencing& new_map_georef);
	
	// Getters
	int getNumSegments() const;
	int getSegmentPointCount(int segment_number) const;
	const TrackPoint& getSegmentPoint(int segment_number, int point_number) const;
	const QString& getSegmentName(int segment_number) const;
	
	int getNumWaypoints() const;
	const TrackPoint& getWaypoint(int number) const;
	const QString& getWaypointName(int number) const;
	
	/// Averages all track coordinates
	LatLon calcAveragePosition() const;
	
	/** Assigns a copy of another Track's data to this object. */
	Track& operator=(const Track& rhs);
	
private:
	bool loadFromGPX(QFile* file, bool project_points);
	
	void projectPoints();
	
	
	std::vector<TrackPoint> waypoints;
	std::vector<QString> waypoint_names;
	
	std::vector<TrackPoint> segment_points;
	// The indices of the first points of every track segment in this track
	std::vector<int> segment_starts;
	std::vector<QString> segment_names;
	
	Georeferencing map_georef;
	
	bool current_segment_finished = true;
};


}  // namespace OpenOrienteering

#endif  // OPENORIENTEERING_TRACK_H
