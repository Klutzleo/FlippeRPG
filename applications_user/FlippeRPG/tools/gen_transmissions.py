#!/usr/bin/env python3
"""Generates narrative/transmissions.c from the lore PDFs, word for word.

    tools/gen_transmissions.py <lore-dir>

<lore-dir> holds FlippeRPG_The_Substrate.pdf and FlippeRPG_Ambient_Transmission_Pool.pdf.
Needs pdftotext (poppler-utils). The lore is kept out of this repo, so this is how the text
gets in. Fails loudly if the PDFs no longer parse into the expected 82 transmissions.
"""
import os, re, subprocess, sys

if len(sys.argv) != 2:
    sys.exit(__doc__)
LORE = sys.argv[1]
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'narrative', 'transmissions.c')

def lines(pdf):
    text = subprocess.run(['pdftotext', os.path.join(LORE, pdf), '-'],
                          check=True, capture_output=True, text=True).stdout
    return [l.strip() for l in text.replace('\f', '\n').split('\n') if l.strip()]

TAG='YOU ARE THE FORCE HIDDEN.'; QR='[SCAN QR TO GO DEEPER]'
def make(block):
    """block = list of non-empty lines. First 3 start with '>' (readout); rest is text."""
    readout=[l for l in block if l.startswith('>')][:3]
    rest=block[len(readout):]
    assert len(readout)==3, block
    assert all(l.startswith('>') for l in block[:3]), block
    threshold = TAG in rest
    text=[l for l in rest if l not in (TAG,QR)]
    return dict(readout=readout,text=text,threshold=threshold)

# ---------- arcs + rescans from The Substrate (document order: RFID, RF, IR, Sub-GHz, NFC, Bluetooth)
L=lines('FlippeRPG_The_Substrate.pdf')
start=next(i for i,l in enumerate(L) if l=='RFID — THE UNREAD' and i+1<len(L) and L[i+1].startswith('TRANSMISSION 1'))
end=next(i for i,l in enumerate(L) if l=='QR LANDING PAGES')
sec=L[start:end]
markers=re.compile(r'^(TRANSMISSION [1-5] — .*|FRIENDLY|ENCOURAGING|PUSHING|[A-Za-z\-]+ RESCAN RESPONSES|[A-Za-z\- ]+ — THE [A-Z ]+|NFC — SYMBIOSIS)$')
blocks=[]; cur=None
for l in sec:
    if markers.match(l):
        if cur: blocks.append(cur)
        cur=[l,[]]
    elif cur: cur[1].append(l)
blocks.append(cur)
arcs=[b for b in blocks if b[0].startswith('TRANSMISSION')]
resc=[b for b in blocks if b[0] in ('FRIENDLY','ENCOURAGING','PUSHING')]
assert len(arcs)==30 and len(resc)==18,(len(arcs),len(resc))
bands=['RFID','RF','IR','SUBGHZ','NFC','BLUETOOTH']
arc={bands[i//5]:[] for i in range(30)}
for i,b in enumerate(arcs): arc[bands[i//5]].append(make(b[1]))
rescan={bands[i//3]:[] for i in range(18)}
for i,b in enumerate(resc): rescan[bands[i//3]].append(make(b[1]))
for b in bands: assert len(arc[b])==5 and len(rescan[b])==3

# ---------- ambient pool
P=lines('FlippeRPG_Ambient_Transmission_Pool.pdf')
amb={}; cur=None; ent=None; order=['RF','IR','SUBGHZ','NFC','BLUETOOTH']; idx=-1
hdr=re.compile(r'^(RF|IR|SUB-GHz|NFC|BLUETOOTH) — [A-Z ]+$')
for l in P:
    if hdr.match(l):
        idx+=1; cur=order[idx]; amb[cur]=[]; ent=None; sub=True; continue
    if cur is None: continue
    if re.match(r'^— \d+ —$',l):
        ent=[]; amb[cur].append(ent); continue
    if ent is None: continue            # section subtitle
    if l.startswith('YOU ARE THE FORCE HIDDEN') : break
    ent.append(l)
for b in order: assert len(amb[b])==10,(b,len(amb[b]))
amb={b:[make(e) for e in amb[b]] for b in order}

# ---------- validation
tier1=['IR','SUBGHZ','NFC','BLUETOOTH']
allx=[t for b in tier1 for t in arc[b]+rescan[b]+amb[b]]+amb['RF']
for t in allx:
    for s in t['readout']+t['text']:
        s.encode('ascii')   # the Flipper font is ASCII only
        assert '"' not in s and '\\' not in s, s
    assert 1<=len(t['text'])<=3, t
for b in tier1: assert [t['threshold'] for t in arc[b]]==[False]*4+[True], b
print('entries:',len(allx),' longest line:',max(len(s) for t in allx for s in t['readout']+t['text']))

def cstr(s): return '"'+s+'"'
def tx(t):
    ro=', '.join(cstr(s) for s in t['readout'])
    tl=t['text']+[None]*(3-len(t['text']))
    te=', '.join(cstr(s) if s else 'NULL' for s in tl)
    return '    {{%s}, {%s}, %s},'%(ro,te,'true' if t['threshold'] else 'false')
def arr(name,dims,groups):
    out=['const Transmission %s%s = {'%(name,dims)]
    for g in groups:
        if isinstance(g,list):
            out.append('    {'); out+= ['    '+tx(t) for t in g]; out.append('    },')
        else: out.append(tx(g))
    out.append('};'); return '\n'.join(out)

head='''// Generated from the lore documents (Transmission Bible / The Substrate / Ambient Transmission
// Pool). Wording is verbatim; do not hand-edit; change the lore and regenerate.
// Band index follows band_order[]: IR, Sub-GHz, NFC, Bluetooth.
#include "transmissions.h"

'''
body=[head]
body.append('// Arc: Ambient, Noticed, Responsive, Urgent, Threshold. Shown once each, in order.')
body.append(arr('tx_arc','[NUM_BANDS][TX_ARC_COUNT]',[arc[b] for b in tier1]))
body.append('\n// Rescan responses: Friendly, Encouraging, Pushing.')
body.append(arr('tx_rescan','[NUM_BANDS][TX_RESCAN_COUNT]',[rescan[b] for b in tier1]))
body.append('\n// Ambient pool: drawn at random once a band\'s arc is complete.')
body.append(arr('tx_ambient','[NUM_BANDS][TX_AMBIENT_COUNT]',[amb[b] for b in tier1]))
body.append('\n// The RF band was merged into Sub-GHz; its ambient entries are the "Ancient Pull" easter egg.')
body.append(arr('tx_ancient_pull','[TX_AMBIENT_COUNT]',amb['RF']))
open(OUT,'w',encoding='ascii').write('\n'.join(body)+'\n')
