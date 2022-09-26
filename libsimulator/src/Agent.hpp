/// Copyright © 2012-2022 Forschungszentrum Jülich GmbH
/// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "Ellipse.hpp"
#include "Journey.hpp"
#include "Line.hpp"
#include "Macros.hpp"
#include "OperationalModel.hpp"
#include "UniqueID.hpp"

#include <memory>

class Agent
{
public:
    using ID = jps::UniqueID<Agent>;
    ID id{};

    // This is evaluated by the "strategic level"
    std::unique_ptr<Behaviour> behaviour{};

    // This is evaluated by the "operational level"
    Point destination{};
    Point waypoint{};
    OperationalModel::ParametersID parameterProfileId;

    /// Desired speed of agent
    double v0;
    Point pos;
    // Gurantueed to be a unit vector
    Point orientation;
    double speed;

private:
    Ellipse _ellipse{}; // the shape of this pedestrian
    Point _e0 = Point(0, 0); // desired direction
    int _newOrientationDelay = 0;

public:
    Agent() = default;
    ~Agent() = default;

    void SetEllipse(const Ellipse& e);
    void SetE0(const Point& p) { _e0 = p; }
    void SetSmoothTurning();
    void IncrementOrientationDelay();

    const Ellipse& GetEllipse() const;
    const Point& GetE0() const;
    Point GetE0(const Point& target, double deltaT) const;
    void InitE0(const Point& target);
    double GetV0() const;
};

std::ostream& operator<<(std::ostream& out, const Agent& pedestrian);
