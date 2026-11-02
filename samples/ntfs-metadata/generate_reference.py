"""Construct only synthetic NTFS3 records in a temporary byte image; never formats a device."""
import base64,hashlib,json,struct,zlib
from pathlib import Path


def put(b,p,fmt,*v):struct.pack_into('<'+fmt,b,p,*v)
def resident(kind,body,identity):
    a=bytearray((24+len(body)+7)//8*8);put(a,0,'II',kind,len(a));put(a,14,'H',identity);put(a,16,'IH',len(body),24);a[24:24+len(body)]=body;return a

def nonresident(runs,size,identity):
    pairs=bytearray();last=0;count=0
    for start,n in runs:
        count+=n
        if start is None:pairs+=bytes((1,n))
        else:
            delta=start-last;raw=delta.to_bytes(2,'little',signed=True);pairs+=bytes((0x21,n))+raw;last=start
    pairs+=b'\0';sparse=any(start is None for start,n in runs);header=72 if sparse else 64;a=bytearray((header+len(pairs)+7)//8*8);put(a,0,'II',0x80,len(a));a[8]=1;put(a,14,'H',identity);put(a,16,'QQH',0,count-1,header);put(a,12,'H',0x8000 if sparse else 0);put(a,40,'QQQ',count*4096,size,size);
    if sparse:put(a,64,'Q',sum(n for start,n in runs if start is not None)*4096)
    a[header:header+len(pairs)]=pairs;return a

def filename(name,identity=0):
    text=name.encode('utf-16le');b=bytearray(66+len(text));put(b,0,'Q',5+(3<<48));
    for p in [8,16,24,32]:put(b,p,'Q',133444736001234567+p)
    b[64]=len(text)//2;b[65]=1;b[66:]=text;return resident(0x30,b,identity)

def record(number,attrs,live=True,directory=False):
    b=bytearray(1024);b[:4]=b'FILE';put(b,4,'HH',48,3);put(b,16,'H',7);put(b,20,'HH',56,int(live)+2*int(directory));put(b,28,'I',1024);put(b,44,'I',number);p=56
    for a in attrs:b[p:p+len(a)]=a;p+=len(a)
    assert p+8<=1024;put(b,p,'I',0xffffffff);put(b,24,'I',p+8);put(b,48,'H',0xa55a)
    for i in range(1,3):b[48+2*i:50+2*i]=b[512*i-2:512*i];put(b,512*i-2,'H',0xa55a)
    return b

def main():
    image=bytearray(4*1024*1024);image[3:11]=b'NTFS    ';put(image,11,'H',512);image[13]=8;put(image,40,'QQQ',len(image)//512,4,1);image[64]=246;image[510:512]=b'\x55\xaa'
    # Fragmented MFT: first12 records at clusters4..6; remaining8 at20..21.
    def offset(i):return 4*4096+i*1024 if i<12 else 20*4096+(i-12)*1024
    def save(i,b):image[offset(i):offset(i)+1024]=b
    save(0,record(0,[nonresident([(4,3),(20,2)],20*1024,0)]))
    volume=bytearray(12);volume[8:10]=b'\x03\x01';save(3,record(3,[resident(0x70,volume,0)]))
    bitmap=bytearray(128)
    for c in [0,1,4,5,6,20,21,100,200]:bitmap[c//8]|=1<<(c%8)
    save(6,record(6,[resident(0x80,bitmap,0)]))
    payload=('بيانات NTFS '.encode('utf-8')*30)+b'abc';assert len(payload)>512
    save(16,record(16,[filename('دليل.bin'),resident(0x80,payload,1)]))
    # Negative relative LCN delta exercises sign extension.
    save(17,record(17,[filename('fragmented.bin'),nonresident([(200,1),(100,1)],6000,1)]))
    save(18,record(18,[filename('deleted.bin'),nonresident([(300,1),(None,1)],6000,1)],live=False))
    save(19,record(19,[filename('folder')],directory=True))
    out={'qCompressBase64':base64.b64encode(struct.pack('>I',len(image))+zlib.compress(image)).decode(),'imageSha256':hashlib.sha256(image).hexdigest(),'residentSha256':hashlib.sha256(payload).hexdigest(),'residentBytes':len(payload),'recordOffsets':{str(i):offset(i) for i in [0,3,6,16,17,18,19]},'expectedNames':['دليل.bin','fragmented.bin','deleted.bin','folder'],'expectedRecords':[16,17,18,19],'scope':'Synthetic known-answer structure, not an independently formatted real NTFS volume'}
    def compressed(b):return base64.b64encode(struct.pack('>I',len(b))+zlib.compress(b)).decode()
    big=bytearray(image);big[4*4096:4*4096+1024]=record(0,[nonresident([(4,33)],129*1024,0)]);out['tooManyRecordsQCompressBase64']=compressed(big)
    many=bytearray(image);many[4*4096:36*4096]=bytes(32*4096)
    many[4*4096:4*4096+1024]=record(0,[nonresident([(4,32)],128*1024,0)])
    for idx,attrs in [(3,[resident(0x70,volume,0)]),(6,[resident(0x80,bitmap,0)])]:many[4*4096+idx*1024:4*4096+(idx+1)*1024]=record(idx,attrs)
    for idx in range(16,128):many[4*4096+idx*1024:4*4096+(idx+1)*1024]=record(idx,[filename('x'*240+str(idx)+'.bin'),resident(0x80,b'',1)])
    out['outputLimitQCompressBase64']=compressed(many)
    Path(__file__).with_name('reference.json').write_text(json.dumps(out,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

if __name__=='__main__':main()
