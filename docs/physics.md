# Rigid body velocities

Use these Squirrel methods on a rigid body created by `_physics.dynamics.createRigidBody`
and added with `_physics.dynamics.addBody`.

| Method | Effect |
|---|---|
| `body.setLinearVelocity(x, y, z)` or `body.setLinearVelocity(vec3)` | Set the velocity of the body's centre along the world axes, in world units per second. |
| `body.getLinearVelocity()` | Read the linear velocity as a `Vec3`. |
| `body.setAngularVelocity(x, y, z)` or `body.setAngularVelocity(vec3)` | Set the spin around the world axes, in radians per second. Wakes a sleeping dynamic body. |
| `body.getAngularVelocity()` | Read the angular velocity as a `Vec3`. |

A read immediately after a setter returns the value just set. After the
physics thread publishes an update for the body, reads use the simulated
value. `_physics.dynamics.shiftOrigin` changes positions while preserving
linear and angular velocity.
