import math
import svgwrite

cx,cy = 300,300 # center
r1 = 70
r2 = 50
numLines = 16
pi = 3.14159265358979323846
sw = 3 # stroke width
sc  = '#A17FFF' # stroke color
aCol  = '#0000FF' # arrow color
opa1 = 0.4 # fill opacity live
opa2 = 0.1 # fill opacity dead



def toDeg(r):
    return r*180/pi


def drawRing(dwg,liveList):
    da = pi*2/numLines
    for i in range(numLines):
        angle = i*da-pi/2
        # dir
        dxA = math.cos(angle)
        dyA = math.sin(angle)
        dxB = math.cos(angle+da)
        dyB = math.sin(angle+da)
        x1 = dxA*r1+cx
        y1 = dyA*r1+cy
        x2 = dxB*r1+cx
        y2 = dyB*r1+cy
        x3 = dxB*r2+cx
        y3 = dyB*r2+cy
        x4 = dxA*r2+cx
        y4 = dyA*r2+cy
        pth = "M {} {} A {} {} 0 0 1 {} {} L {} {} A {} {} 0 0 0 {} {} Z".format(
            x1,y1,       # move to
            r1,r1,x2,y2, # arc to
            x3,y3,       # line to
            r2,r2,x4,y4) # arc to
        if i in liveList:
            opp = opa1
        else:
            opp = opa2
        dwg.add(dwg.path(d=pth, stroke=sc,stroke_width=sw,fill=sc,fill_opacity=opp))

def create_arrow_marker(dwg):
    arrow = dwg.marker(id='arrow', insert=(0, 1), size=(10, 10), orient='auto', markerUnits='strokeWidth')
    arrow.add(dwg.path(d='M0,0 L0,2 L3,1 z', fill=aCol))
    dwg.defs.add(arrow)
    return arrow

def drawArrow(dwg,index):
    d = pi*2/numLines
    angle = index*pi*2/numLines+d/2-pi/2
    dr = r1-r2
    r3,r4=r1+0.5*(dr),r1+dr
    # perhaps see markers section of api
    x = math.cos(angle)
    y = math.sin(angle)

    x1,y1 = cx+r3*x,cy+r3*y
    x2,y2 = cx+r4*x,cy+r4*y

    line = dwg.line(start=(x2,y2),end=(x1,y1),stroke=aCol,stroke_width=sw)
    arrow = create_arrow_marker(dwg)
    line['marker-end'] = arrow.get_funciri()
    #line.set_markers((arrow,arrow,arrow))
    dwg.add(line)

    r5 = (r3+r4)/2
    x3,y3 = r5*math.cos(angle)+cx,r5*math.sin(angle)+cy
    angle += pi/8
    x4,y4 = r5*math.cos(angle)+cx,r5*math.sin(angle)+cy
    pth = "M {} {} A {} {} 0 0 1 {} {}".format(
        x3,y3,       # move to
        r5,r5,x4,y4 # arc to
        ) 
    p = dwg.path(d=pth, stroke=aCol,stroke_width=sw,fill_opacity=0.0)
    p['marker-end'] = arrow.get_funciri()
    dwg.add(p)





# txt = text to draw
# index = start pos (fractional), is mid angle of cell
# alpha - fraction of (r1-r2) from r2 to draw
def drawArcText(dwg,txt,index,alpha,bold=False):
    r  = r2 + alpha*(r1-r2)

    angle = index*pi*2/numLines + pi/numLines - pi/2
    x1 = math.cos(angle)*r+cx
    y1 = math.sin(angle)*r+cx

    angle += pi*1.9
    x2 = math.cos(angle)*r+cx
    y2 = math.sin(angle)*r+cx

    pth = "M {} {} A {} {} 0 1 1 {} {} Z".format(x1,y1,r,r,x2,y2)
    w = dwg.path(d=pth, stroke=sc,stroke_width=0,fill_opacity=0.0)
    dwg.add(w)
    text = dwg.add(svgwrite.text.Text(""))
    text.add(svgwrite.text.TextPath(path=w, text=txt, font_family='consolas', startOffset=None, method='align', spacing='exact'))


def drawText(dwg,alpha,txt,bold=False):
    index = 0
    d = 2*pi/numLines-0.2
    #d /= 2
    for i in range(len(txt)):
        sTxt = txt[i]
        drawArcText(dwg,sTxt,index-d,alpha)
        index += 1

    #if len(lastTxt) > 0:
    #    d = (3+len(lastTxt))*d

def makeRing(ind1,ind2,filename,contents=True,contentsText = 'ABCDEFGHIJK'):
    dwg = svgwrite.Drawing(filename)

    fillEnd = ind2+1
    if ind2+1 < ind1:
        fillEnd += numLines
    lst = [i%numLines for i in range(ind1,fillEnd)]
    drawRing(dwg,lst)
    
    drawArrow(dwg,ind1)   
    drawArcText(dwg,"next read",ind1,2.1)
    dd = 0
    if ind2+1 == ind1 or ind2+2 == ind1:
        dd = 1
    drawArrow(dwg,ind2+1)   
    drawArcText(dwg,"next write",ind2+1,2.1+dd)
    if contents:
        drawText(dwg,0.2,contentsText,True)    
    drawText(dwg,1.2,'0123  6789. ...')    
    drawArcText(dwg,'N-1',(numLines-1)-2.2*pi/numLines,1.2)
    dwg.save()

makeRing(4,10,'RingBuffer.svg')    
makeRing(2,1,'EmptyRingBuffer.svg',False)    
makeRing(2,0,'FullRingBuffer.svg',True,'Q CDEFGHIJKLMNOP')    


