#ifndef PARTICLE_SOURCE_H
#define PARTICLE_SOURCE_H
#pragma once

#include "globalincs/pstypes.h"

#include "object/object.h"

#include "particle/particle.h"
#include "particle/ParticleEffect.h"

#include "io/timer.h"

struct weapon;

struct weapon_info;

// Forward declaration so we don't need weapons.h
enum class WeaponState: uint32_t;

namespace particle {
/**
 * The origin type
 */
enum class SourceOriginType {
	NONE, //!< Invalid origin
	VECTOR, //!< World-space offset
	OBJECT, //!< An object
	PARTICLE //!< A particle
};

class ParticleSource;

class ParticleEffect;

/**
 * @brief A source origin
 *
 * The SourceOrigin class encapsualtes the information about where a source is located. It allows a source to be
 * relative to an object or a particle and keeps track of whether the host objects are still valid.
 */
class SourceOrigin {
 private:
	SourceOriginType m_originType;

	struct {
		vec3d m_pos;

		object_h m_object;

		WeakParticlePtr m_particle;
	} m_origin;

	WeaponState m_weaponState;

	vec3d m_offset;

 public:
	/**
	 * @brief Initializes the origin with default values
	 */
	SourceOrigin();

	/**
	 * @brief Gets the current, global position of the origin
	 * @param posOut The pointer where the location will be stored
	 */
	void getGlobalPosition(vec3d* posOut) const;

	inline SourceOriginType getType() const { return m_originType; }

	inline object* getObjectHost() const { return m_origin.m_object.objp; }

	/**
	 * @brief Determines if the origin is valid
	 *
	 * This checks if the hosting object is valid or if the hosting particle is still valid.
	 *
	 * @return @c true if the origin is valid, @c false otherwise
	 */
	bool isValid() const;

	/**
	 * @brief Applies the information to a particle info
	 *
	 * This can be used to initialize the position of a created particle from a source.
	 *
	 * @param info The particle_info this should be applied to
	 * @param allowRelative If @c true then the location of the particle can be relative to the host
	 */
	void applyToParticleInfo(particle_info& info, bool allowRelative = false) const;

	/**
	 * @brief Gets the velocity of the origin host
	 * @return The velocity of the host
	 */
	vec3d getVelocity() const;

	/**
	 * @brief Sets the weapon state in which this origin is valid
	 * @param state The weapon state to use
	 */
	void setWeaponState(WeaponState state);

	/**
	 * @brief Moves the source to the specified world location
	 * @param pos The world position
	 */
	void moveTo(vec3d* pos);

	/**
	 * @brief Moves the source to the specified object with an offset
	 * @param objp The hosting object
	 * @param offset The position relative to this object
	 */
	void moveToObject(object* objp, vec3d* offset);

	/**
	 * @brief Moves the source to the specified particle
	 * @param weakParticlePtr The hosting particle
	 */
	void moveToParticle(WeakParticlePtr weakParticlePtr);

	friend class ParticleSource;
};

/**
 * @brief The orientation of a particle source
 *
 * Currently only the forward direction vector is useful because the other vectors of the matrix are chosen pretty
 * arbitrarily. This also contains a normal vector if it was specified when creating the source.
 */
class SourceOrientation {
 private:
	matrix m_orientation;

	bool m_hasNormal = false;
	vec3d m_normal;

 public:
	SourceOrientation();

	/**
	 * @brief Sets the direction from a vector
	 *
	 * The vector doesn't have to be normalized before passing it to this function.
	 *
	 * @param vec The vector to create the matrix from
	 */
	void setFromVector(const vec3d& vec);

	/**
	 * @brief Sets the direction from an already normalized vector
	 * @param vec The normalized vector
	 */
	void setFromNormalizedVector(const vec3d& vec);

	void setNormal(const vec3d& normal);

	void setFromMatrix(const matrix& mat);

	vec3d getDirectionVector() const;

	/**
	 * @brief Gets the normal of this orientation
	 * @param outNormal The pointer where the normal will be written
	 * @return @c true if there was a normal, @c false otherwise
	 */
	bool getNormal(vec3d* outNormal) const;

	inline const matrix* getMatrix() const { return &m_orientation; }

	friend class ParticleSource;
};

/**
 * @brief Contains information about the timing of a source
 *
 * This can be used to control when a source is active and when it will be deleted. Since a source can be active at a
 * later time the timing goes through three stages. In the first state the source is created but not active. #isActive()
 * returns false but #isFinished() also return false. When it activates #isActive() will return true while #isFinished()
 * stays @c false. Then #isActive() will be @c false and #isFinished() will be @c true. At this point the source is
 * considered to have expired and it will be removed the next time it is processed.
 */
class SourceTiming {
 private:
	int m_creationTimestamp;

	int m_beginTimestamp;
	int m_endTimestamp;

 public:
	SourceTiming();

	void setCreationTimestamp(int time);

	int getCreationTime() const { return m_creationTimestamp; }

	/**
	 * @brief Sets when the source is active
	 *
	 * The source will be active between the specified begin and end times
	 *
	 * @param begin The timestamp when the source will begin to be active
	 * @param end The timestamp when the source will not be active anymore
	 */
	void setLifetime(int begin, int end);

	/**
	 * @brief Determines if the source is currently active
	 * @return @c true if the source is active
	 */
	bool isActive() const;

	/**
	 * @brief Determines if the source has expired
	 * @return @c true if the source has expired.
	 */
	bool isFinished() const;

	/**
	 * @brief Gets the progress of the source through its active time.
	 *
	 * If the source is not active or if the timestamps are not valid then -1 is returned
	 *
	 * @return The progress of the source through its lifetime
	 */
	float getLifeTimeProgress() const;

	friend class ParticleSource;
};

/**
 * @brief A particle source
 *
 * A particle source contains the information about where and for how long particles are created. A particle effect uses
 * this information to create new particles. A particle source has not effect-specific information which means that an
 * effect can only use the information contained in this object.
 *
 * @ingroup particleSystems
 */
class ParticleSource {
 private:
	SourceOrigin m_origin; //!< The current position of this particle source

	SourceOrientation m_effectOrientation; //!< The orientation of the particle source

	SourceTiming m_timing; //!< The time informations of the particle source

	ParticleEffect* m_effect; //!< The effect that is assigned to this source

	size_t m_processingCount; //!< The number of times this effect has been processed

	void initializeThrusterOffset(weapon* wp, weapon_info* wip);
 public:
	ParticleSource();

	inline const ParticleEffect* getEffect() const { return m_effect; }

	inline ParticleEffect* getEffect() { return m_effect; }

	inline void setEffect(ParticleEffect* eff) {
		Assert(eff != nullptr);
		m_effect = eff;
	}

	inline const SourceOrigin* getOrigin() const { return &m_origin; }

	inline SourceOrigin* getOrigin() { return &m_origin; }

	inline const SourceOrientation* getOrientation() const { return &m_effectOrientation; }

	inline SourceOrientation* getOrientation() { return &m_effectOrientation; }

	inline const SourceTiming* getTiming() const { return &m_timing; }

	inline SourceTiming* getTiming() { return &m_timing; }

	inline size_t getProcessingCount() const { return m_processingCount; }

	/**
	 * @brief Finishes the creation of a particle source
	 *
	 * This is used to initialize some status that is only available after everything has been set up.
	 */
	void finishCreation();

	/**
	 * @brief Does one processing step for this source
	 * @return @c true if the source should continue to be processed
	 */
	bool process();

	/**
	 * @brief Determines if the source is valid
	 * @return @c true if the source is valid, @c false otherwise.
	 */
	bool isValid() const;
};
}


#endif // PARTICLE_SOURCE_H

