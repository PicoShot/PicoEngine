# PicoEngine — agent orientation

## Project identity and direction

PicoEngine is a C++ game engine, not just a rendering library. It is being built by
two Unity game developers as a private alternative to Unity for their own games.
Their Unity experience is the starting point for the engine's design and usage.

The goal is **advanced engine systems with easy-to-use, Unity-like C# APIs**.
Gameplay should feel familiar: scenes contain game objects, objects have
transforms and components, and components participate in a managed lifecycle.
Rendering, physics, audio, animation, input, assets, and UI should work together
through that model wherever it makes sense.

Unity is a reference for the developer experience, not a requirement for exact
API compatibility, feature parity, or identical internals. Preserve familiar
concepts and workflows while choosing implementations appropriate to this engine.
Do not assume Unity packages or Unity runtime APIs are available here.

This repository is actively evolving. This document describes its enduring
intent and broad organization; it is not a feature inventory, API specification,
or implementation roadmap. Read the current source before making assumptions
about a system's capabilities or limitations.

## Build and dependencies

- **All engine libraries must be statically linked. No DLLs.**
if possible external libraries must staticly linked to executable, no dynamic libraries as much as posible

## Design guidance

- **Design from gameplay usage.** For new APIs, consider the code a game developer
  would write for an ordinary use case. Keep common operations straightforward,
  give sensible defaults, and expose advanced controls when needed.
- **Make engine features reusable.** A capability demonstrated through custom
  sandbox code may still need integration into the engine's normal scene,
  component, asset, and authoring workflows. Trace that integration before
  declaring a feature complete.
- **Keep complexity behind the appropriate boundary.** Gameplay-facing APIs
  should not require native graphics/audio handles or backend-specific setup
  unless the caller deliberately uses a low-level API. Keep platform, backend,
  and content-processing concerns in their appropriate layers.
- **Respect the component model.** Consider activation, lifecycle order,
  hierarchy changes, deferred destruction, and scene transitions when extending
  a system. Do not introduce a parallel gameplay architecture or broad rewrite
  without a concrete need in the task.
- **Consider persistence and ownership.** New state and references may need to
  survive saving/loading or participate in asset lifetime management. Inspect
  current serialization and ownership conventions rather than assuming automatic
  field serialization or garbage collection handles native resources.
- **Treat performance as part of usability.** Avoid unnecessary allocation,
  blocking I/O, and repeated expensive work in frame/fixed-update paths. Keep
  thread and resource-lifetime requirements clear. Use measurements to justify
  more complex optimization.

These principles guide implementation choices; they do not require building
every related feature during a narrowly scoped task. Keep changes focused on
the requested outcome and account for the integration points it actually touches.
