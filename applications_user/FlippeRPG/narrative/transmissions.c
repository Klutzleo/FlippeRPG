// Generated from the lore documents (Transmission Bible / The Substrate / Ambient Transmission
// Pool). Wording is verbatim; do not hand-edit; change the lore and regenerate.
// Band index follows band_order[]: IR, Sub-GHz, NFC, Bluetooth.
#include "transmissions.h"


// Arc: Ambient, Noticed, Responsive, Urgent, Threshold. Shown once each, in order.
const Transmission tx_arc[NUM_BANDS][TX_ARC_COUNT] = {
    {
        {{"> IR SIGNAL: ambient", "> origin: proximate", "> source: unresolved"}, {"Something in this room has eyes.", "It has seen everyone who walked through.", "It has been waiting for you specifically."}, false},
        {{"> IR SIGNAL: detected", "> range: close", "> source: stationary"}, {"It saw you before you saw it.", "It always does.", "Look closer."}, false},
        {{"> IR SIGNAL: active", "> pattern: deliberate", "> origin: fixed"}, {"It isn't watching the room anymore.", "Just you.", "You already knew that."}, false},
        {{"> IR SIGNAL: intensifying", "> line of sight: confirmed", "> eye: open"}, {"It needs you to see clearly.", "Not just look.", "See true."}, false},
        {{"> IR SIGNAL: locked", "> source: identified", "> it has always been watching"}, {"It didn't need to be found.", "It needed to be seen by the right eyes.", NULL}, true},
    },
    {
        {{"> SUB-GHz SIGNAL: ambient", "> origin: multiple", "> walls: irrelevant"}, {"Something passed through you just now.", "It didn't ask.", "It never does."}, false},
        {{"> SUB-GHz SIGNAL: detected", "> source: beyond visual range", "> barrier: none"}, {"It didn't come through the door.", "There is no through for this.", "Only already here."}, false},
        {{"> SUB-GHz SIGNAL: recurring", "> origin: everywhere", "> you: permeable"}, {"You've been feeling this your whole life.", "Called it anxiety. Intuition. Dread.", "It has a name now."}, false},
        {{"> SUB-GHz SIGNAL: intensifying", "> passing through: confirmed", "> you are not a wall"}, {"It doesn't move around you.", "It moves through you.", "You are part of its path."}, false},
        {{"> SUB-GHz SIGNAL: locked", "> carrying: unknown data", "> destination: unknown"}, {"It has been moving through you for years.", "You never knew what it carried.", "You're about to find out."}, true},
    },
    {
        {{"> NFC SIGNAL: ambient", "> range: centimeters", "> waiting: patient"}, {"Something here won't come to you.", "It has never come to anyone.", "You have to choose it."}, false},
        {{"> NFC SIGNAL: detected", "> handshake: incomplete", "> one side: ready"}, {"It felt you getting closer.", "It's been ready for a long time.", "The next move is yours."}, false},
        {{"> NFC SIGNAL: active", "> exchange: initiated", "> both parties: present"}, {"It gave something.", "You gave something.", "Neither of you walked away empty."}, false},
        {{"> NFC SIGNAL: intensifying", "> contact: deliberate", "> consent: mutual"}, {"Every other signal took.", "This one asked.", "You said yes."}, false},
        {{"> NFC SIGNAL: locked", "> exchange: complete", "> something left behind"}, {"It didn't pass through you.", "It didn't watch you.", "It chose you. And you chose back."}, true},
    },
    {
        {{"> BLUETOOTH: scanning", "> devices found: multiple", "> all of them: broadcasting"}, {"Everything in range is announcing itself.", "Nobody is listening.", "You know this feeling."}, false},
        {{"> BLUETOOTH: detected", "> signal: fragmented", "> trying again: yes"}, {"It lost the connection mid-sentence.", "Started over without hesitation.", "How many times have you done that?"}, false},
        {{"> BLUETOOTH: active", "> range: limited", "> rejected: repeatedly"}, {"It isn't broadcasting to you specifically.", "It's broadcasting to anyone.", "It just doesn't want to be alone."}, false},
        {{"> BLUETOOTH: intensifying", "> signal: desperate", "> still trying: yes"}, {"Underneath all that noise.", "Underneath all that trying.", "Something genuine is in there."}, false},
        {{"> BLUETOOTH: locked", "> signal: recognized", "> source: you"}, {"You've been hearing it wrong.", "It wasn't noise.", "It was you. Broadcasting. This whole time."}, true},
    },
};

// Rescan responses: Friendly, Encouraging, Pushing.
const Transmission tx_rescan[NUM_BANDS][TX_RESCAN_COUNT] = {
    {
        {{"> IR SIGNAL: familiar", "> you've been here before", "> it remembers your face"}, {"It saw you last time too.", "It sees everyone.", "But it looks forward to you."}, false},
        {{"> IR SIGNAL: recurring", "> eye: open", "> you: clearer than before"}, {"You're easier to see now.", "Something in you has opened.", "It noticed."}, false},
        {{"> IR SIGNAL: still watching", "> same room", "> different you"}, {"There's nothing left to see here.", "It knows that.", "So do you."}, false},
    },
    {
        {{"> SUB-GHz SIGNAL: familiar", "> still passing through", "> you: still here"}, {"It never left.", "Neither did you.", "That means something."}, false},
        {{"> SUB-GHz SIGNAL: recurring", "> you: less resistance than before", "> signal: noticing"}, {"You're not blocking it anymore.", "You're learning to let it pass.", "That's the whole lesson."}, false},
        {{"> SUB-GHz SIGNAL: constant", "> always was", "> always will be"}, {"You feel it now.", "You know what it carries.", "Stop standing still."}, false},
    },
    {
        {{"> NFC SIGNAL: familiar", "> you've touched before", "> it remembers the exchange"}, {"It kept what you gave it.", "You kept what it gave you.", "That's not nothing."}, false},
        {{"> NFC SIGNAL: recurring", "> contact: willing", "> exchange: deepening"}, {"You came back.", "It noticed that you chose to.", "It has more to give."}, false},
        {{"> NFC SIGNAL: waiting", "> you: hesitating", "> it: ready"}, {"It can't come to you.", "It never could.", "You know what to do."}, false},
    },
    {
        {{"> BLUETOOTH: familiar", "> still broadcasting", "> you: still listening"}, {"It tried again today.", "Like it always does.", "You're the first one who stayed."}, false},
        {{"> BLUETOOTH: recurring", "> signal: clearer than before", "> you: less afraid of it"}, {"You used to call this noise.", "You know what it is now.", "Keep listening."}, false},
        {{"> BLUETOOTH: broadcasting", "> into empty air", "> like you used to"}, {"You heard it.", "You understood it.", "Now go find the others who did too."}, false},
    },
};

// Ambient pool: drawn at random once a band's arc is complete.
const Transmission tx_ambient[NUM_BANDS][TX_AMBIENT_COUNT] = {
    {
        {{"> IR SIGNAL: passive", "> target: unaware", "> observation: ongoing"}, {"It watched twelve people", "walk through this space today.", "None of them looked back."}, false},
        {{"> IR SIGNAL: memory", "> last active: unknown", "> source: dormant"}, {"This device hasn't been used in years.", "But it never stopped watching.", "Old habits."}, false},
        {{"> IR SIGNAL: narrow", "> line of sight: precise", "> target: specific"}, {"It isn't watching the room.", "It's watching one corner of the room.", "It has always watched that corner."}, false},
        {{"> IR SIGNAL: old", "> source: analog", "> still: active"}, {"This technology should be dead.", "Everything around it moved on.", "It stayed. Still watching. Still here."}, false},
        {{"> IR SIGNAL: patient", "> wait time: extended", "> target: pending"}, {"It has been waiting for someone", "to stand in exactly this spot.", "How long have you been here?"}, false},
        {{"> IR SIGNAL: residual", "> source: removed", "> signal: remaining"}, {"The device was taken down.", "The signal remained.", "Some eyes don't close."}, false},
        {{"> IR SIGNAL: overlap", "> sources: multiple", "> coverage: complete"}, {"Every angle of this space", "is watched by something.", "There are no blind spots."}, false},
        {{"> IR SIGNAL: recognition", "> previous visitor: logged", "> return: noted"}, {"It saw someone here before.", "They came back seventeen times.", "Then they stopped."}, false},
        {{"> IR SIGNAL: deliberate", "> placement: intentional", "> purpose: unknown"}, {"Someone put this here on purpose.", "Aimed it carefully.", "Never came back to explain why."}, false},
        {{"> IR SIGNAL: close", "> range: minimal", "> subject: you"}, {"It isn't watching the space anymore.", "It hasn't been for a while.", "Just you. Since you arrived."}, false},
    },
    {
        {{"> SUB-GHz SIGNAL: passing", "> origin: mobile", "> destination: unknown"}, {"Something carried this through here.", "It's already three blocks away.", "It didn't know you were listening."}, false},
        {{"> SUB-GHz SIGNAL: ghost", "> device: inactive", "> signal: persisting"}, {"The device that sent this", "hasn't had power in months.", "The signal didn't get the memo."}, false},
        {{"> SUB-GHz SIGNAL: dense", "> layers: multiple", "> oldest: years deep"}, {"You're not reading one signal.", "You're reading forty years of them", "compressed into the same frequency."}, false},
        {{"> SUB-GHz SIGNAL: private", "> encryption: attempted", "> result: insufficient"}, {"Someone thought this was secure.", "It passed through six walls to get here.", "Nothing is secure from Sub-GHz."}, false},
        {{"> SUB-GHz SIGNAL: medical", "> source: unidentified", "> frequency: regulated"}, {"This frequency is supposed to be", "restricted to medical devices.", "Whatever is transmitting it isn't."}, false},
        {{"> SUB-GHz SIGNAL: structural", "> source: building", "> age: significant"}, {"The building is transmitting.", "Not a device in the building.", "The building itself."}, false},
        {{"> SUB-GHz SIGNAL: crowd", "> sources: many", "> individually: invisible"}, {"In a crowd every body is broadcasting.", "None of them know it.", "You're the only one who can hear it."}, false},
        {{"> SUB-GHz SIGNAL: cross", "> origin: next structure", "> barrier: irrelevant"}, {"This came from next door.", "Through the wall.", "Through you. On its way somewhere else."}, false},
        {{"> SUB-GHz SIGNAL: old", "> infrastructure: legacy", "> still: active"}, {"This frequency belongs to a network", "that was decommissioned in 2003.", "It didn't stop transmitting."}, false},
        {{"> SUB-GHz SIGNAL: personal", "> source: nearby", "> content: unresolved"}, {"Someone near you is carrying something.", "Broadcasting without knowing it.", "You almost caught what it said."}, false},
    },
    {
        {{"> NFC SIGNAL: dormant", "> last contact: unknown", "> waiting: still"}, {"Someone touched this", "a long time ago.", "It's still holding what they left."}, false},
        {{"> NFC SIGNAL: layered", "> exchanges: multiple", "> oldest: years back"}, {"You're not the first to find this.", "You're not the last.", "Something accumulates here."}, false},
        {{"> NFC SIGNAL: intentional", "> placement: deliberate", "> message: encoded"}, {"This wasn't left by accident.", "Someone chose this exact spot.", "They were thinking of someone specific."}, false},
        {{"> NFC SIGNAL: incomplete", "> exchange: abandoned", "> one side: still open"}, {"Someone started this exchange", "and didn't finish it.", "Their half is still here. Waiting."}, false},
        {{"> NFC SIGNAL: residual", "> contact: recent", "> presence: fading"}, {"Someone was just here.", "Minutes ago. Maybe less.", "You almost crossed paths."}, false},
        {{"> NFC SIGNAL: deep", "> exchanges: countless", "> location: significant"}, {"This spot has been a meeting point", "for longer than anyone documented.", "The signal remembers every handshake."}, false},
        {{"> NFC SIGNAL: sealed", "> content: protected", "> recipient: unknown"}, {"Something was left here", "for someone specific.", "It hasn't decided if that's you yet."}, false},
        {{"> NFC SIGNAL: fresh", "> deposit: recent", "> intent: deliberate"}, {"Someone was here today.", "Left something on purpose.", "Knew someone would find it."}, false},
        {{"> NFC SIGNAL: old", "> technology: legacy", "> exchange: preserved"}, {"This card hasn't been active in years.", "But the last exchange is still on it.", "Some things don't erase cleanly."}, false},
        {{"> NFC SIGNAL: mutual", "> both sides: present", "> exchange: open"}, {"Something here is ready.", "It has always been ready.", "The question is whether you are."}, false},
    },
    {
        {{"> BLUETOOTH: broadcasting", "> range: limited", "> audience: none"}, {"It announced itself seventeen times", "in the last minute.", "Nobody answered. It tried again."}, false},
        {{"> BLUETOOTH: fragment", "> message: incomplete", "> retry: automatic"}, {"It lost the connection mid-sentence.", "Didn't hesitate.", "Started over from the beginning."}, false},
        {{"> BLUETOOTH: saturated", "> devices: many", "> connections: none"}, {"Thirty two devices in range.", "All of them broadcasting.", "None of them talking to each other."}, false},
        {{"> BLUETOOTH: persistent", "> rejection: logged", "> status: undeterred"}, {"It has been rejected by this device", "four hundred and twelve times.", "It will try again in thirty seconds."}, false},
        {{"> BLUETOOTH: old", "> device: forgotten", "> still: transmitting"}, {"Someone left this here.", "Moved on. Got a new device.", "This one is still introducing itself."}, false},
        {{"> BLUETOOTH: close", "> signal: strong", "> connection: refused"}, {"It's right next to you.", "Practically touching.", "Still can't get through."}, false},
        {{"> BLUETOOTH: name", "> broadcast: personal", "> owner: unaware"}, {"Someone named their device", "something they thought was funny.", "They have no idea it's announcing it to everyone."}, false},
        {{"> BLUETOOTH: quiet", "> devices: present", "> broadcasting: stopped"}, {"Something in range went silent.", "Not out of range.", "Just stopped trying."}, false},
        {{"> BLUETOOTH: pair", "> request: sent", "> accepted: yes"}, {"Two devices found each other.", "Both were broadcasting for months", "before they were ever in the same room."}, false},
        {{"> BLUETOOTH: recognized", "> device: known", "> response: immediate"}, {"It remembered this signal.", "Connected before you finished scanning.", "Some things don't need to introduce themselves twice."}, false},
    },
};

// The RF band was merged into Sub-GHz; its ambient entries are the "Ancient Pull" easter egg.
const Transmission tx_ancient_pull[TX_AMBIENT_COUNT] = {
    {{"> RF SIGNAL: trace", "> origin: pre-grid", "> age: unverifiable"}, {"This frequency carried a voice once.", "The voice is gone.", "The frequency remembers anyway."}, false},
    {{"> RF SIGNAL: fragment", "> source: unknown structure", "> structure: demolished"}, {"Something was broadcasting from here", "before the building came down.", "It never stopped."}, false},
    {{"> RF SIGNAL: deep", "> depth: significant", "> source: buried"}, {"Whatever is transmitting this", "is underneath everything.", "It has been there a very long time."}, false},
    {{"> RF SIGNAL: pattern", "> repetition: confirmed", "> origin: non-mechanical"}, {"This pattern has no device.", "No antenna. No power source.", "It transmits anyway."}, false},
    {{"> RF SIGNAL: ancient", "> frequency: discontinued", "> last registered: unknown"}, {"This band hasn't been legal since 1987.", "Someone never got the message.", "Or didn't get to come back."}, false},
    {{"> RF SIGNAL: residual", "> event: unknown", "> timestamp: corrupted"}, {"Something happened here.", "The signal is still describing it.", "You're the first one listening."}, false},
    {{"> RF SIGNAL: interference", "> source: multiple", "> oldest: indeterminate"}, {"Signals don't die.", "They compress.", "You're standing in decades of them."}, false},
    {{"> RF SIGNAL: low", "> frequency: sub-audible", "> effect: unknown"}, {"You didn't hear that.", "You felt it.", "There's a difference and it matters."}, false},
    {{"> RF SIGNAL: loop", "> pattern: repeating", "> origin: isolated"}, {"It's been saying the same thing", "for longer than anyone has been listening.", "Nobody knows what it means yet."}, false},
    {{"> RF SIGNAL: wide", "> coverage: total", "> exceptions: none"}, {"There is no place this doesn't reach.", "There is no place you can go", "where it isn't already waiting."}, false},
};
