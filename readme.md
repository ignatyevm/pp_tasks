<h1>Merge sort</h1>
Example:
<blockquote>mpiexec.exe -n 8 merge_sort.exe -(mode) (input) [optional flags]</blockquote>
Mode can be <code>random</code> or <code>file</code> <br/>
If mode is <code>random</code> , program generate random array with <code>input</code> elements (from <code>INT_MIN</code> to <code>INT_MAX</code>) <br/>
If mode is <code>file</code>, program read array from <code>input</code> file
Optional flags:
<ul>
<li><code>-time</code> - print time of execution</li>
<li><code>-print</code> - print sorted array</li>
</ul>
<h4>Some examples:</h4>
<blockquote>mpiexec.exe -n 8 merge_sort.exe -random 1000 -time</blockquote> - generate random array with 1000 elemtns and print time of execution<br/>
<blockquote>mpiexec.exe -n 8 merge_sort.exe -file input.txt -time -print</blockquote> - read array from input.txt, print time of execution and sorted array

<h1>Floyd algorithm</h1>
<blockquote>mpiexec.exe -n 8 floyd_algorithm.exe -(mode) (input) [optional flags]</blockquote>
Same flags as in <code>merge_sort.exe</code> <br/>
But, in input file first number mean size of adjacency, next adjacency matrix <br/>
As well, <code>-random</code> generate complete graph on <code>n</code> vertices with random weights <br/>
Some experiment:
<table>
<thead>
<th>n</th>
<th>1 process</th>
<th>2 processes</th>
<th>4 processes</th>
<th>8 processes</th>
</thead>
<tbody>
<tr>
<td>100</td><td>0.0539973</td><td>0.0309079</td><td>0.0316805</td><td>0.0486019</td>
</tr>
<tr>
<td>500</td><td>3.44302</td><td>2.34767</td><td>1.81729</td><td>2.61638</td>
</tr>
<tr>
<td>1000</td><td>29.7395</td><td>17.278</td><td>14.4528</td><td>19.0871</td>
</tr>
</tbody>
</table>
So, 4 processes work quickly then 8 because on my machine 4 physics cores and 8 logical cores, and in this moment on my machine works IDE, Browser and other Windows 10 software. <br/>
For measure of time of execution i used the MPI function <code>MPI_Wtime()</code>
<h1>Determinant</h1>
<blockquote>mpiexec.exe -n 8 determinant.exe -(mode) (input) [optional flags]</blockquote>
Same as in <code>Merge sort</code> and <code>Floyd algorithm</code>