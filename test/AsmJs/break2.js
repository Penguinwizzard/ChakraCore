  function AsmModule() {
    "use asm";      
    var x1 = 10;
    function f3(x,y){
        x = x|0;
        y = +y;
        var m = 1000;
        var n = 10;
        var z = 11;

       a: for( m = 0; (m|0) < 50 ; m = (m+1)|0)
        {
            x = (x+1)|0
            if( (x|0) > 10)
            {
                while(1)
                {
                    if((n|0) > 50)
                        break a;
                    x = (x+1)|0;
                    z = (z+1)|0;
                    n = (n+1)|0;
                }
            }            
        }
        return (x + z)|0;
    }
    
    return f3
}

var f3 = AsmModule();
print(f3  (1,1.5))  
print(f3  (1,1.5))   