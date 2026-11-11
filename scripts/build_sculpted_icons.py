"""Author deterministic, unframed metal/enamel MR SVG symbols (no raster/emoji)."""
from pathlib import Path
import hashlib,json,math
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'assets/icons-professional'
DEFS='''<defs>
<linearGradient id="silver" x1="0" y1="0" x2=".8" y2="1"><stop stop-color="#FCFEFF"/><stop offset=".19" stop-color="#B0C4D4"/><stop offset=".43" stop-color="#F7FCFF"/><stop offset=".48" stop-color="#8CA3B8"/><stop offset=".7" stop-color="#C8D7E1"/><stop offset="1" stop-color="#536A82"/></linearGradient>
<linearGradient id="edge" x2=".5" y2="1"><stop stop-color="#F2FAFF"/><stop offset=".44" stop-color="#9DB8CD"/><stop offset="1" stop-color="#324A65"/></linearGradient>
<linearGradient id="blue" x1=".1" y1="0" x2=".8" y2="1"><stop stop-color="#AAB8FF"/><stop offset=".26" stop-color="#657EE6"/><stop offset=".5" stop-color="#344DAD"/><stop offset=".55" stop-color="#203578"/><stop offset="1" stop-color="#101D43"/></linearGradient>
<linearGradient id="red" x2=".6" y2="1"><stop stop-color="#FFE2EA"/><stop offset=".26" stop-color="#FF526E"/><stop offset=".52" stop-color="#FF1744"/><stop offset="1" stop-color="#A80030"/></linearGradient>
<linearGradient id="glass" x2="1" y2="1"><stop stop-color="#244C69"/><stop offset=".45" stop-color="#0D2239"/><stop offset=".5" stop-color="#071424"/><stop offset="1" stop-color="#132F48"/></linearGradient>
<linearGradient id="green" x2=".5" y2="1"><stop stop-color="#E0FFEC"/><stop offset=".5" stop-color="#39FF14"/><stop offset="1" stop-color="#078B28"/></linearGradient>
<linearGradient id="amber" x2=".5" y2="1"><stop stop-color="#FFF3C7"/><stop offset=".4" stop-color="#D7A34A"/><stop offset="1" stop-color="#75511D"/></linearGradient>
</defs>'''
def path(d,fill='silver',stroke='edge',sw=.8,extra=''):
 return f'<path d="{d}" fill="url(#{fill})" stroke="url(#{stroke})" stroke-width="{sw}" stroke-linejoin="round" {extra}/>'
def rect(x,y,w,h,r=2,fill='silver',stroke='edge',sw=.8):
 return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{r}" fill="url(#{fill})" stroke="url(#{stroke})" stroke-width="{sw}"/>'
def circle(x,y,r,fill='silver',sw=.8):
 return f'<circle cx="{x}" cy="{y}" r="{r}" fill="url(#{fill})" stroke="url(#edge)" stroke-width="{sw}"/>'
def line(d,color='#B8D4EB',sw=1,opacity=1):
 return f'<path d="{d}" fill="none" stroke="{color}" stroke-width="{sw}" opacity="{opacity}" stroke-linecap="round" stroke-linejoin="round"/>'
def screw(x,y):return circle(x,y,1.2,'glass',.3)+line(f'M{x-.6} {y}h1.2','#A5BACB',.5)
def doc():return path('M17 7H38L49 18V54H17Z')+path('M38 7V18H49','blue')+rect(22,23,19,2,1,'blue',sw=0)+rect(22,29,15,1.3,.6,'glass',sw=0)+rect(22,34,17,1.3,.6,'glass',sw=0)
def lock(x=34,y=35):
 return line(f'M{x+4} {y+5}v-5a6 6 0 0 1 12 0v5','#162A43',4)+line(f'M{x+4} {y+4}v-5a6 6 0 0 1 12 0v5','#DCEAF5',2)+rect(x,y+3,20,16,3,'red')+circle(x+10,y+10,2,'glass',.4)+line(f'M{x+10} {y+11}v3','#041020',1.4)
icons={}
icons['home']=path('M10 29L32 10 54 29V55H37V39H27V55H10Z')+path('M6 29L32 6 58 29 54 34 32 15 10 34Z','blue')+rect(15,32,8,11,1,'glass')+rect(41,32,8,11,1,'glass')+line('M19 33v9m-3-4h6M45 33v9m-3-4h6','#87BBD7',.7)+line('M12 53h13M39 53h12','#F7FCFF',1)+circle(35,46,1,'green',.1)
icons['document_lock']=doc()+lock()
icons['clipboard_task']=rect(13,10,39,46,4,'blue')+rect(17,14,31,38,2)+rect(24,7,17,9,3,'glass')+rect(27,6,11,4,2)+line('M22 26l3 3 5-6M22 39l3 3 5-6','#0A6532',3.3)+line('M22 26l3 3 5-6M22 39l3 3 5-6','#39FF14',1.7)+line('M34 26h9M34 29h6M34 39h9M34 42h6','#48617C',1.1)+screw(17,53)+screw(48,53)
icons['phone']=rect(19,5,28,53,5)+rect(22,10,22,40,2,'glass')+path('M23 11H43V24L23 40Z','blue',sw=0)+line('M28 8h10','#253C54',1.5)+circle(33,54,1.6,'glass')+line('M24 41l5-7 4 4 7-13','#39FF14',1.6)+circle(40,25,1.7,'red')
icons['evidence_box']=path('M8 24L31 14 56 24V50L32 59 8 50Z')+path('M8 24L32 34 56 24 31 14Z','blue')+path('M32 34V59L56 50V24Z','glass')+path('M24 17L47 28V35L40 38V30L18 20Z','red')+rect(13,34,13,10,1,'glass')+line('M16 37h7m-7 3h5','#BACBD8',.8)+lock(34,34)
icons['history']=path('M12 24A24 24 0 1 1 9 40L15 37A18 18 0 1 0 18 25L23 30 6 32 7 15Z')+circle(33,32,17,'glass')+line('M33 19v14l9 5','#AABEFF',2.5)+circle(33,33,2,'red')+''.join(line(f'M{33+14*math.sin(a):.2f} {32-14*math.cos(a):.2f}l{1.8*math.sin(a):.2f} {-1.8*math.cos(a):.2f}','#8FA9BE',.8) for a in [i*math.pi/6 for i in range(12)])
icons['shield_checkmark']=path('M32 5C40 11 47 13 56 14V31C55 44 44 54 32 59 20 54 9 44 8 31V14C17 13 24 11 32 5Z')+path('M32 11C39 16 45 18 50 18V31C49 40 41 48 32 53 23 48 15 40 14 31V18C20 17 26 15 32 11Z','blue')+line('M19 20v11c0 7 7 14 13 17','#91A9EE',.9,.6)+path('M23 31L29 37 42 23 46 27 29 44 19 35Z','silver')+circle(46,48,9,'green')+line('M42 48l3 3 6-7','#E7FFF5',1.8)
icons['book_open_lightbulb']=path('M5 18C15 14 24 16 32 21 40 16 49 14 59 18V53C49 50 40 52 32 57 24 52 15 50 5 53Z','blue')+path('M8 15C17 12 25 15 32 19V52C24 48 16 46 8 49Z')+path('M32 19C39 15 47 12 56 15V49C48 46 40 48 32 52Z')+line('M32 20v31','#405976',1.4)+line('M13 24l13 3m-13 3 13 3m-13 3 13 3M38 28l13-4m-13 10 13-4','#4B6685',1)+circle(46,15,9,'blue')+line('M42 14a4 4 0 1 1 7 3l-1 3h-4l-1-3z','#39FF14',1.8)+rect(44,22,4,2,1,'silver',sw=0)
icons['training_lab']=path('M21 8H43V14H39V29L54 51Q56 57 50 58H14Q8 57 10 51L25 29V14H21Z')+path('M29 15H35V31L47 51H17L29 31Z','glass')+path('M23 39H41L49 53H15Z','blue')+circle(27,44,2,'red')+circle(35,48,1.8)+circle(32,36,1.3)+line('M18 50h5m0-5h3M26 10h12','#F4FAFF',1)+screw(24,12)+screw(40,12)
icons['document_text']=doc()+rect(23,40,4,9,1,'blue')+rect(30,37,4,12,1,'blue')+rect(37,41,4,8,1,'red')+line('M22 51H43','#607C93',.8)
# precision machined gear, with integrated indigo hub
gear=[]
for i in range(48):
 a=2*math.pi*i/48-math.pi/2;r=27 if i%6 in (1,2,3,4) else 22
 gear.append(f'{32+r*math.cos(a):.3f},{32+r*math.sin(a):.3f}')
icons['settings']=f'<polygon points="{" ".join(gear)}" fill="url(#silver)" stroke="url(#edge)" stroke-width=".8"/>'+circle(32,32,16,'blue')+circle(32,32,10,'silver')+circle(32,32,5,'glass')+''.join(screw(32+20*math.cos(a),32+20*math.sin(a)) for a in [i*math.pi/2+math.pi/4 for i in range(4)])
icons['toolbox']=path('M23 19V11Q23 7 27 7H39Q43 7 43 11V19H38V12H28V19Z','glass')+rect(7,20,51,35,5,'blue')+rect(7,19,51,13,4)+line('M9 33H56','#010E22',1.2)+rect(17,28,5,13,1,'green')+rect(43,28,5,13,1,'red')+rect(25,39,16,8,1,'glass')+line('M29 43h8','#BBD3E1',1)+screw(11,49)+screw(54,49)
icons['phone_laptop']=rect(5,9,42,32,3)+rect(8,12,36,25,1,'glass')+path('M4 43H49L54 50H2Z','blue')+rect(20,44,14,3,1)+rect(39,21,19,35,3)+rect(42,25,13,25,1,'glass')+line('M45 33l3-3 4 4m-7 9 7-7','#9AB3FA',1.3)+circle(49,53,1,'red',.1)
icons['arrow_square_down']=rect(9,7,46,48,6,'blue')+rect(12,10,40,41,3,'glass')+path('M28 16H37V32H44L32 44 20 32H28Z')+line('M20 48H44','#D5E8F4',1.5)+screw(14,13)+screw(50,13)
icons['gauge']=path('M7 46A27 27 0 1 1 57 46L49 51H15Z')+path('M13 43A21 21 0 1 1 51 43L46 46H18Z','glass')+line('M17 37l-4 1m7-13-3-3m15-4v-4m12 11 3-3m1 15 4 1','#AABEE9',1.5)+path('M29 38L46 20 35 42Z','red')+circle(32,40,4)+rect(24,48,16,5,1,'blue')
icons['board']=rect(7,8,50,47,4,'blue')+rect(21,20,22,22,2,'silver')+rect(25,24,14,14,1,'glass')+line('M12 17h6v10h3M12 47h6v-10h3M43 25h8v-9M43 37h8v10M28 20v-6h-5M36 42v8','#A3B9CC',1.4)+circle(12,17,2,'red')+circle(51,47,2,'red')+screw(11,12)+screw(53,12)+screw(11,51)+screw(53,51)
icons['arrow_sync']=path('M11 23A24 24 0 0 1 52 16L56 12 58 29 41 27 46 22A17 17 0 0 0 17 26Z')+path('M53 41A24 24 0 0 1 12 48L8 52 6 35 23 37 18 42A17 17 0 0 0 47 38Z','blue')+circle(32,32,5,'glass')
icons['wrench']=path('M35 8Q45 3 54 9L43 18 46 25 54 26 60 17Q62 35 46 37L22 59Q16 63 10 57 6 52 11 46L34 25Q29 15 35 8Z')+path('M16 48L37 28 40 31 19 52Z','blue')+circle(16,53,2.5,'glass')+line('M36 10l-1 9m8 16 10-5','#F4FAFF',1)
icons['data_bar_vertical_ascending']=path('M8 7H12V51H58V56H8Z')+rect(17,35,9,13,1,'blue')+rect(31,25,9,23,1,'blue')+rect(45,12,9,36,1,'red')+line('M18 36h6m8-10h6m8-13h6','#D8E6FF',.8)
icons['search_visual']=circle(27,26,20)+circle(27,26,16,'glass')+path('M39 42L46 36 61 51 54 58Z','blue')+circle(26,25,7,'blue')+circle(26,25,3,'glass')+line('M14 25H20M32 25H39M26 13V19M26 31V38','#AEC6E5',1.2)+line('M16 17q8-8 17-2','#E4F3FF',1,.7)
icons['database']=rect(10,15,44,34,2,'blue')+path('M10 36C10 48 54 48 54 36V47C54 59 10 59 10 47Z')+path('M10 24C10 36 54 36 54 24V35C54 47 10 47 10 35Z','blue')+'<ellipse cx="32" cy="15" rx="22" ry="10" fill="url(#silver)" stroke="url(#edge)" stroke-width=".8"/>'+'<ellipse cx="32" cy="14" rx="16" ry="5" fill="url(#glass)"/>'+circle(47,39,1.7,'red')+circle(47,49,1.7,'green')
icons['puzzle_piece']=path('M10 13H25C19 2 39 2 33 13H47V27C58 21 62 40 47 36V51H33C40 62 20 62 25 51H10V36C22 42 22 22 10 27Z','blue')+line('M13 16H25M13 40V48H23M36 16H44V23','#C1D0FF',1,.7)+circle(35,33,5,'silver')
for name,fill in [('checkmark_circle','green'),('error_circle','red'),('question_circle','blue')]:
 body=circle(32,31,26)+circle(32,31,21,fill)
 if name=='checkmark_circle':body+=path('M18 30L27 39 46 20 50 24 27 47 14 34Z')
 elif name=='error_circle':body+=path('M22 18L32 28 42 18 46 22 36 32 46 42 42 46 32 36 22 46 18 42 28 32 18 22Z')
 else:body+=line('M24 24a8 8 0 1 1 12 7q-5 3-5 7','#E5F1FF',4)+circle(31,45,2.5)
 icons[name]=body
icons['warning']=path('M29 7Q32 3 35 7L60 50Q62 55 57 56H7Q2 55 4 50Z','amber')+path('M31 16H34L35 37H30Z','glass')+circle(32.5,46,3,'glass')+line('M8 51L29 13','#FFF8DF',1,.6)
icons['check_mark']=line('M10 31L25 46 55 15','#D6E6F3',6)
OUT.mkdir(exist_ok=True)
items=[]
for name,body in icons.items():
 svg=f'<svg xmlns="http://www.w3.org/2000/svg" width="64" height="64" viewBox="0 0 64 64">{DEFS}<ellipse cx="32" cy="59.5" rx="22" ry="2" fill="#000" opacity=".28"/><g transform="translate(0 1.5)" opacity=".8">{body}</g><g>{body}</g></svg>\n'
 p=OUT/(name+'.svg');p.write_text(svg);items.append({'file':p.name,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
(OUT/'PROVENANCE.json').write_text(json.dumps({'author':'Nexvary MR independently authored vector material system','treatment':'Unframed sculpted silver, indigo enamel, neon red #FF1744 and neon green #39FF14; original geometry authored by scripts/build_sculpted_icons.py. No emoji or library glyph tracing. Semantic green/amber/red status accents retained.','license':'Project license; no upstream geometry in these generated assets','files':items},indent=2)+'\n')
print(f'Built {len(items)} sculpted metal SVG icons')
