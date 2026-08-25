# ChatGPT Conversation Mode / Voice on Linux

**Verified 2026-08-21. Original scope: ordinary ChatGPT voice conversation. The
2026-08-21 addendum below separately covers the user's actual follow-up: attach
normal, interruptible Voice to an already-running Codex task on Linux.**

## The short answer

Yes.  The official ChatGPT desktop app now has a Linux **preview**, and OpenAI's
current Voice documentation says desktop Voice supports natural turn-taking and
interruptions.  But for this machine, the best way to get exactly the requested
experience today is simpler: use the official [ChatGPT website](https://chatgpt.com/)
in a normal browser and press the Voice button.  It is official, does not need a
Linux package, and OpenAI explicitly documents Live as listening while it speaks,
so you can interrupt it naturally.

The local machine is Kali GNU/Linux Rolling 2026.3, x86_64, GNOME/Wayland. Kali
is Debian-derived but is **not** one of OpenAI's formally supported desktop-app
distributions. Do not treat a Debian package installing successfully as official
Kali support.

## What is officially available

| Need | Answer |
|---|---|
| Official Linux desktop app | **Yes, preview.** Announced 2026-08-11. |
| Official ordinary ChatGPT Voice on Linux | **Yes.** The official web experience is documented. The current desktop Voice documentation also describes normal Chat, not only Work/Codex. |
| Natural interruption | **Yes, with Live.** Live listens and speaks at once. |
| Supported desktop-app distributions | Ubuntu 24.04 LTS / 26.04 LTS, Debian 13, Fedora 43 / 44. x64 and ARM64 packages. |
| Kali Linux | Not formally supported. The web method is the supported practical route; the `.deb` is a reasonable experiment, not a promised setup. |

OpenAI calls the modern mode **Live**. It is the one that has the normal
conversation behaviour: you speak, it speaks, and either side can begin talking
without waiting for the other to finish. Standard Voice is not the same thing;
it transcribes your whole turn before responding.

## Best choices

| Rank | Method | Does it meet the goal? | Catch |
|---:|---|---|---|
| 1 | **Official ChatGPT in Firefox/Chrome/Chromium** | **Yes.** Same ChatGPT account, history, voices, Live, and interruption behaviour. | No global hotkey/background phone-style mode. Voice availability still depends on account, region, and rollout. |
| 2 | Official ChatGPT Linux desktop app preview | **Yes, when Voice is enabled for the account.** Native app; its docs explicitly say Voice in Chat supports interruption. | Kali is unsupported; the preview's global Voice hotkey has a reported Linux/Wayland bug. Starting Voice from the on-screen button is the practical workaround. |
| 3 | [VoxInput](https://github.com/richiejp/VoxInput) community project | **Mostly.** Its OpenAI Realtime API mode explicitly supports two-way audio and "barge-in" interruption. | It is a separate API-key application, not ChatGPT. It will not use the normal ChatGPT subscription, account history, memory, or official Voice UI. It is terminal-first. |

Another community option, [ChatGTK](https://github.com/rabfulton/ChatGTK), advertises
OpenAI Realtime voice in a GTK app. It is more GUI-like, but it also needs an API
key and its own chat history. It is therefore not a replacement for ChatGPT
Conversation Mode. It has no reason to be installed unless the official choices
fail and a separate API assistant is acceptable.

## Recommendation: do this first on Kali

1. Open a current Firefox, Chrome, or Chromium window.
2. Go to [chatgpt.com](https://chatgpt.com/) and sign in to the ordinary personal
   ChatGPT account.
3. Click the **Voice** icon at the right of the message box.
4. When the browser asks, choose **Allow microphone**.
5. If ChatGPT asks, choose a voice. In ChatGPT **Settings → Voice**, select
   **Live** if it is offered.
6. Speak. While it is answering, start speaking again to interrupt it.

If the Voice icon is missing, first make sure the account is in an eligible
region and that Voice is available to its plan. OpenAI says availability can vary
by plan, region, workspace settings, and rollout. A personal account is the
right target for this test. If Live is not listed, the account may only have
Advanced or Standard Voice yet; neither proves Linux is the problem.

Use headphones if the microphone hears ChatGPT's own speakers. That is the
single most common reason a voice conversation keeps cutting itself off.

## Correction: starting Voice in an existing Codex chat

An earlier version of this note said that a Codex chat must be created in Voice
first. That was too definite. OpenAI's newer Help Center article (updated four
days ago when checked) gives the operative instructions: open the desktop app,
choose **Codex**, select the **Voice control**, allow the microphone, and begin
speaking. It does not say that the currently open Codex chat must be replaced.

So try the Voice control in the existing chat first. If it opens a live session,
the chat is now Voice-enabled and interruption works normally. If this Linux
preview build only presents dictation or has no Voice control, do not infer that
the chat itself is permanently incompatible: the official docs are presently
inconsistent. The initial release note says "Start a new task in Voice", and a
separate documentation page says a task must begin in Voice. That is a rollout
or documentation gap which OpenAI has not clearly resolved for Linux preview.

## Live troubleshooting check, 2026-08-21

The installed app on this Kali machine is current (`26.814.41407`). The machine
has a working PipeWire service and a digital microphone. Its default speaker
output was found **muted** and was unmuted. Therefore, test Voice again before
installing anything else.

The important visual distinction is:

| What the control does | What it is | Does ChatGPT speak back? |
|---|---|---|
| Records speech, writes it into the composer, then waits for Send | Dictation | No |
| Opens a live call/listening screen or waveform with an End control | Voice chat | Yes |

There is no confirmed community workaround that turns dictation into the full
Voice chat on the official Linux preview. Recent reports confirm that the global
Voice hotkey is broken on Linux/Wayland, but that is a start-button problem, not
an audio-output workaround. The reliable next action is to use the distinct
live Voice control, then inspect the app's audio stream only while the live
session is open.

## Existing Codex task: attach Voice instead of creating a second task

This was researched again after the initial conclusion. The correct target is
the **existing task's composer**, not the app-wide "Start new voice chat" entry.
A recent desktop-app issue records the successful sequence as: open an existing
task, then start **Voice Chat from that task's composer**. It skips creating a
provisional new task and starts faster. The public Codex app-server protocol
also calls this a **thread-scoped** realtime session and accepts a particular
`threadId` with `outputModality: "audio"`; it is architecturally able to attach
to an existing task.

Practical steps:

1. Leave the desired existing Codex task open.
2. Hover the audio controls in *that task's composer*. Click only the control
   whose tooltip says **Start Voice Chat** (or equivalent), not **Dictate** /
   voice input.
3. The result must show a live-call state and an End control. Ask it to describe
   the current task to prove it retained this task's context.

This is not a new-thread workaround; it is the correct thread-attachment path.
**Machine verification:** the currently installed Linux app (`26.814.41407`)
exposes only **Dictate** in an existing task; it does not expose Start Voice
Chat. The Windows path above is real but is not available in this Linux-preview
UI. Therefore it must not be presented as a working Kali instruction.

The caveat is Linux preview maturity. A fresh Reddit post reports the same
missing-existing-thread limitation, and no public Linux-specific flag or safe
config workaround was found. A third-party `vl-talk` tool claims per-session
voice attachment, but its author says it is Windows-only, so it is not a Kali
solution. The public protocol proves a custom Linux client could be built, but
there is no maintained, drop-in client that attaches it to this official desktop
task with ChatGPT-account Voice.

## Optional: install the official app later

Only do this after the browser test works. On a supported Debian/Ubuntu x64
system, download the official x64 `.deb` from OpenAI's Linux page, then run:

```sh
cd ~/Downloads
sudo apt install ./chatgpt_amd64.deb
chatgpt
```

The package adds OpenAI's signed package repository. Later updates are:

```sh
sudo apt update
sudo apt install --only-upgrade chatgpt
```

For Fedora, use the official `.rpm` and `sudo dnf install ./chatgpt.x86_64.rpm`.
On Wayland, OpenAI says the Linux preview normally uses XWayland; native Wayland
is still experimental. This is unrelated to whether on-screen Voice works.

## Evidence checked

### Official OpenAI sources

- [ChatGPT Voice help](https://help.openai.com/en/articles/20001274) — current
  Live/Advanced/Standard definitions; web start steps; Live interruption; account
  and rollout limits.
- [ChatGPT Linux desktop-app documentation](https://learn.chatgpt.com/docs/linux/linux-app)
  — preview status, supported distributions, architectures, official packages,
  install/update commands, and Wayland limitation.
- [ChatGPT desktop Voice documentation](https://learn.chatgpt.com/docs/features/voice)
  — Voice in ordinary Chat, natural interruption, and desktop Voice requirements.
- [ChatGPT Work and Codex help](https://help.openai.com/en/articles/20001275/)
  — newer, direct steps for Voice in Codex: select the Voice control, permit the
  microphone, then speak. This is the source used for the correction above.
- [OpenAI community announcement](https://community.openai.com/t/codex-desktop-app-for-linux-is-now-in-preview/1390027)
  — August 11 Linux-preview announcement.

### Community cross-checks

- [OpenAI GitHub issue #38126](https://github.com/openai/codex/issues/38126) —
  an open August 2026 report that Linux/Wayland global Voice hotkeys do not
  register. This affects starting Voice by keyboard, not the in-app Voice button.
- [VoxInput README](https://github.com/richiejp/VoxInput) — documents Realtime
  assistant mode and explicit barge-in interruption.
- [ChatGTK README](https://github.com/rabfulton/ChatGTK) — documents GTK
  Realtime Voice, but requires an API key and separate configuration.
- Recent [r/ChatGPT Linux discussion](https://www.reddit.com/r/ChatGPT/comments/1vselwp/chatgpt_on_linux/)
  and [r/archlinux packaging discussion](https://www.reddit.com/r/archlinux/comments/1vmd1lj/official_chatgpt_app_was_announced_today_and_the/)
  independently confirm the very recent Linux release, while also showing why
  the official OpenAI package is safer than random community wrappers.
- [Existing-task Voice Chat investigation](https://github.com/openai/codex/issues/36587)
  — documents a successful "open an existing task, start Voice Chat from its
  composer" path, and its faster thread-attached start.
- [Official Codex app-server protocol](https://github.com/openai/codex/blob/main/codex-rs/app-server/README.md)
  — documents `thread/realtime/start` as an audio-capable, thread-scoped
  realtime session.
- [Fresh existing-chat request from another user](https://www.reddit.com/r/codex/comments/1vjfies/voice_in_codex_should_be_attachable_to_existing/)
  — evidence that the visible UI/rollout remains inconsistent; its suggested
  third-party tool is Windows-only.

## Weakest link

The Linux desktop app is only days old and still preview software. The browser
route is the safer recommendation because it uses the official, documented web
Voice implementation and does not depend on Kali compatibility or the Linux
hotkey bug.

---

# Addendum: make full Voice work with an existing Codex task on Linux

## The real answer

There is **no ready-to-install Linux application today** that can take the
already-open task in the Linux Codex desktop preview (where this machine shows
only **Dictate**) and add the missing two-way Voice button.

But there is a real, supported-by-code route to make it. It does **not** mean
gluing together speech-to-text and a random text-to-speech app. Codex itself
has an experimental local Voice protocol which is explicitly tied to a
particular Codex thread:

1. Resume the existing task's `threadId`.
2. Start `thread/realtime/start` with audio output and a WebRTC microphone
   connection.
3. Play the returned WebRTC audio stream.

That is the normal Voice shape: you talk, Codex talks, and interrupting is part
of the realtime connection. The official protocol documentation gives this
exact browser/WebView WebRTC flow, including `getUserMedia`, an audio track,
the returned SDP, and `outputModality: "audio"`.

This Kali machine's installed `codex-cli 0.147.0` was checked locally. Its
generated experimental app-server schema contains both `thread/resume` and
`thread/realtime/start`, including the WebRTC audio transport. So the local
runtime needed for a Linux client is present. No credentials were read or
changed during that check.

## The one I recommend

**Port the open-source [Jarvis x Codex](https://github.com/Big-Guan/jarvis-codex)
client to Linux, with an existing-task picker.**

That project is the closest thing to the requested result already built. It is
a small Tauri desktop app, not a fake assistant: it resumes a Codex thread,
opens the official Codex Voice V3 WebRTC connection, uses the local Codex login,
and keeps voice, text, tools, and work in that one thread. Its author has
verified the full loop on macOS. A separate Windows fork has verified the same
Voice backend, which proves the Mac-only part is the wake-word shell, not the
actual Codex Voice path.

For Linux, remove the macOS/Windows wake-word pieces initially. Open the app,
choose your existing task, press **Talk**, and it should provide precisely the
conversation mode you asked for. A wake word can be added later; it is not
needed for normal voice conversation.

## Simple comparison

| Method | Same existing Codex task? | Natural talk-back + interrupt? | Linux today? | Verdict |
|---|---:|---:|---:|---|
| Official Linux Codex preview | No on this machine: only Dictate is visible | No; Dictate is input only | Yes | Not enough. |
| **Linux port of Jarvis x Codex** | **Yes, by resuming the selected thread** | **Yes; WebRTC Codex Voice** | Build required; no released Linux package | **Best route.** |
| [Brokk Codex ACP](https://github.com/BrokkAi/brokk-codex-acp) | Yes, it can list/resume threads | Not yet: it reports audio as diagnostics and explicitly defers native playback | Its release plan includes Linux | Useful protocol/reference code, not a voice app. |
| `vl-talk` community tool | Per-session voice is claimed | Claims spoken output | Windows-only, per its author | Not usable on Kali. |
| ChatGTK, VoxInput, local Whisper/TTS apps | No; separate conversation/API identity | Some can interrupt | Often yes | Reject: these do not continue *this* Codex task. |

## What the Linux port has to do

This is a contained desktop-client job, not a rewrite of Codex:

1. Start the local `codex app-server` with its experimental realtime feature.
2. Show a list of local Codex tasks and let you select one. For an already-open
   task, use its stored thread ID; never create a replacement task.
3. Ask the app for microphone permission.
4. Use the WebView's normal WebRTC audio APIs to connect that thread's Voice
   session and play its returned audio.
5. Provide **Talk / Stop**. Talking over Codex is the normal interruption.

The upstream project already implements steps 1, 3, 4, and 5. Its current
source stores and resumes a thread ID per workspace. The Linux work is mainly
packaging, removing platform-only wake code, and adding a proper existing-task
picker instead of only its one-thread-per-folder rule.

## What is verified versus not yet proven

**Verified:** the official protocol is thread-scoped and supports WebRTC audio;
the Mac project implements it; the Windows adaptation verified its Voice
backend; and this machine's installed Codex runtime includes the required
methods.

**Not yet proven:** a released Linux build, and simultaneous use of exactly the
same task from the Linux preview window and the new Voice client. The safe test
is to resume one existing non-critical task in the Linux voice client, speak a
short message, then check that both the spoken turn and response are preserved
in the same task history. If Codex rejects two live writers to one task, the
client must be the active view while Voice is running; it can still be the same
task, not a duplicate.

## Evidence for this addendum

- [Official Codex app-server protocol](https://github.com/openai/codex/blob/main/codex-rs/app-server/README.md)
  documents `thread/resume` and the experimental, thread-scoped
  `thread/realtime/start` WebRTC audio path.
- [Jarvis x Codex](https://github.com/Big-Guan/jarvis-codex) documents a local
  client that keeps voice, text, tool activity, and work in one Codex thread;
  its source resumes a thread then starts Voice through that protocol.
- [Windows Jarvis adaptation](https://github.com/Kuhakucai/Jarvis) reports the
  same realtime Voice backend working on Windows, reducing the platform risk
  to desktop/wake integration rather than Codex Voice itself.
- [Brokk Codex ACP](https://github.com/BrokkAi/brokk-codex-acp) independently
  implements thread lifecycle/realtime protocol access on Linux but documents
  that native realtime audio playback is still missing.
- Recent [Codex community discussion](https://www.reddit.com/r/codex/comments/1vjfies/voice_in_codex_should_be_attachable_to_existing/)
  confirms the exact visible-UI limitation: people want Voice attached to an
  existing chat, while a cited workaround is Windows-only.
