# S32K Naming Review

## Scope

This review covers the three S32K project folders in the current workspace:

- `S32K_Can_slave`
- `S32K_LinCan_master`
- `S32K_Lin_slave`

The goal is to check whether names match actual behavior, whether the naming system is internally consistent, and which names should be kept or changed first.

## One-line Assessment

The codebase style is mostly good at the file and API level, but the naming vocabulary is mixed in three places:

- project and folder names
- node and role terminology
- a small set of high-impact function and state names

The biggest issue is not formatting style. The biggest issue is that one concept is often described by more than one word.

## Overall Verdict

### What is already good

- File names are mostly consistent lower snake case.
- Macro names are mostly consistent upper snake case.
- Internal task names like `button`, `can`, `led`, `lin_fast`, `lin_poll` are readable.
- Many state variables match behavior well, especially debounce and retry fields.

### What is not fully consistent

- `master` and `coordinator` are both used for the same application-side node.
- `slave1`, `field node`, and `slave2`, `sensor slave` are mixed.
- `lock`, `latch`, `clear`, and `release` are used for the same state family.
- `fresh` is overloaded and does not mean one thing consistently.

## Project Folder Naming

### Current names

| Current folder | Main issue | Comment |
| --- | --- | --- |
| `S32K_Can_slave` | protocol and role order differs from the others | readable, but not part of one clear pattern |
| `S32K_LinCan_master` | mixed protocol order and mixed case chunking | readable, but the naming rule is hard to infer |
| `S32K_Lin_slave` | closer to the other slave folder, but still not aligned with the master folder | readable, but not standardized |

### Recommended direction

Pick one primary naming system and use it everywhere.

### Preferred system: meaning-first application roles

- `S32K_Coordinator_LIN_CAN`
- `S32K_FieldNode_CAN`
- `S32K_SensorNode_LIN`

This is the clearest option because it separates application role from bus role.

### Conservative system: keep existing node numbering model

- `S32K_Master_LIN_CAN`
- `S32K_Slave1_CAN`
- `S32K_Slave2_LIN`

This is easier if the project already depends heavily on `slave1` and `slave2`.

## Naming Rule For Roles

Use one word set per concept.

### Recommended vocabulary split

- Application role:
  - `Coordinator`
  - `FieldNode`
  - `SensorNode`
- Bus role:
  - `LIN master`
  - `LIN slave`
- Address or legacy node mapping:
  - `slave1`
  - `slave2`

### Important rule

Do not use `master` and `coordinator` interchangeably unless the code is explicitly talking about the LIN bus role.

## Verdict On `linfast`

### Final judgment

`lin_fast` is acceptable as an internal scheduler task name.

`linfast` is not recommended as a standalone folder, module, or product-level name.

### Why

- In the current code, `lin_fast` is paired with `lin_poll`, so the relative meaning is understandable.
- As a standalone name, `linfast` is ambiguous:
  - fast scheduler tick
  - fast path
  - high-speed LIN
  - optimized LIN driver

### Recommendation

- Keep `lin_fast` if it remains an internal task name.
- Avoid `linfast`.
- If the real role is event handling, prefer `lin_event` or `lin_event_task`.

## Function And Variable Review

### Keep as-is

These names match actual behavior well.

| Name | Verdict | Reason |
| --- | --- | --- |
| `button_last_sample_pressed` | keep | clearly describes the debounce input sample |
| `button_same_sample_count` | keep | clearly describes repeated identical samples |
| `button_stable_pressed` | keep | clearly describes debounced stable state |
| `ok_tx_pending` | keep | correctly means OK token TX is queued but not completed |
| `ok_token_pending` | keep | correctly means a received OK token is waiting to be consumed |
| `started_ms` | keep | accurate retry-flow start time |
| `last_retry_ms` | keep | accurate retry timing marker |
| `retry_count` | keep | accurate retry counter |
| `AppSlave1_TaskButton` | keep | behavior and name match closely |
| `AppSlave2_HandleLinOkToken` | keep | behavior and name match closely |
| `AdcModule_ClearEmergencyLatch` | keep | behavior and name match closely |

### Rename recommended

These names are understandable, but they do not describe the real behavior precisely enough.

| Current name | Problem | Better direction |
| --- | --- | --- |
| `AppCore_RequestSlave1Ok` | it does not send anything; it only marks a pending request state | `AppCore_MarkSlave1OkPending` or `AppCore_ReserveSlave1OkRequest` |
| `AppMaster_RequestOk` | it starts a full approval flow, not just a simple request | `AppMaster_StartOkApprovalFlow` |
| `LinModule_RequestOk` | it only queues OK token transmission for the next poll slot | `LinModule_QueueOkTokenTx` |
| `master_emergency_active` | it also covers fault and latched states, not only emergency zone | `master_block_active` or `lin_alarm_active` |
| `can_task_count` | it increments only when there was CAN activity, not on every task run | `can_active_tick_count` |
| `lin_last_reported_lock` | the rest of the code uses latch terminology | `lin_last_reported_latch` |
| `AppMaster_IsFaultOrEmergencyStatus` | it also returns true for latched states that are not currently emergency zone | `AppMaster_IsBlockingLinStatus` |

### Strong rename recommendation

These names are the most likely to confuse future readers.

| Current name | Problem | Better direction |
| --- | --- | --- |
| `LinModule_GetLatestStatusIfFresh` | it checks age/timestamp freshness, not the `fresh` field itself | `LinModule_GetLatestStatusIfRecent` or `LinModule_GetLatestStatusWithinAge` |
| `LinStatusFrame.fresh` | it is overloaded: wire flag, unconsumed state, and newness are mixed together | split semantics or rename to `unconsumed` |
| `AppMaster_blockOkRelayForEmergency` | inconsistent casing and the behavior is broader than only emergency | `AppMaster_BlockOkRelayForAlarm` or `AppMaster_BlockOkRelayForBlockingState` |

## Naming Conflict: `fresh`

`fresh` is currently doing too much work.

### Current meanings mixed together

- newly received over LIN
- not yet consumed by upper-layer logic
- still within allowed age window
- encoded status bit on the wire

### Why this is risky

The code has both:

- a `fresh` field in `LinStatusFrame`
- a timestamp-based age check via `latest_status_rx_ms`

That means a reader can easily assume the wrong thing when they see `GetLatestStatusIfFresh` or `ConsumeFreshStatus`.

### Better model

Use two separate concepts:

- `unconsumed` or `has_new_status`
- timestamp-based recency via `latest_status_rx_ms`

If the wire protocol still needs a `fresh` bit, keep it at protocol boundary only and do not reuse it as the main application meaning.

## Naming Conflict: `lock` vs `latch`

The state is structurally a latch, but some UI and cache names use `lock`.

### Recommendation

Use `latch` consistently in code:

- `emergency_latched`
- `last_reported_latch`
- `ClearEmergencyLatch`
- `latch clear pending`

If UI text must stay short, `lock` can be used only as presentation text. It should not be the main code term.

## Recommended Naming Rules

### Rule 1

One concept should have one primary noun.

Examples:

- use `latch`, not both `lock` and `latch`
- use `Coordinator`, not both `Coordinator` and `Master` for the same application role

### Rule 2

If a function only marks state, do not name it like it already performs I/O.

Examples:

- `Mark...Pending`
- `Queue...`
- `Start...Flow`

### Rule 3

If a function checks age, say `recent`, `within_age`, or `timeout`, not `fresh`.

### Rule 4

Use `fast` only for scheduler cadence, not for product-level naming.

Good:

- `lin_fast`

Avoid:

- `linfast`

### Rule 5

Application role names and protocol role names should not share the same label unless that is truly the same concept.

## Suggested Priority

### Priority 1

Lock the shared vocabulary first.

- coordinator or master
- field node or slave1
- sensor node or slave2
- latch or lock

### Priority 2

Rename the misleading function and state names.

- `LinModule_GetLatestStatusIfFresh`
- `fresh`
- `master_emergency_active`
- `can_task_count`
- `RequestOk` chain

### Priority 3

Rename folder and project names after the internal vocabulary is stable.

This reduces churn and avoids renaming everything twice.

## Recommended Final Direction

If this project will continue to grow, the most maintainable naming system is:

- Application roles:
  - `Coordinator`
  - `FieldNode`
  - `SensorNode`
- Bus roles:
  - `LIN master`
  - `LIN slave`
- Internal scheduler tasks:
  - `lin_fast`
  - `lin_poll`
- Avoid as standalone names:
  - `linfast`

## Short Conclusion

The codebase does not have a general naming quality problem.

It has a vocabulary consistency problem.

If only a few names are cleaned up, the payoff will be high:

- fix `fresh`
- fix the `RequestOk` chain
- stop mixing `master` and `coordinator`
- stop mixing `lock` and `latch`

